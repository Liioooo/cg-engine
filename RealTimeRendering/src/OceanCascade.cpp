#include <Logging.h>
#include "OceanCascade.h"
#include "CgEngine/Rendering/GraphicsObjectsFactory.h"
#include "CgEngine/Rendering/Renderer.h"

namespace RTR {
    OceanCascade::OceanCascade(const OceanParams& oceanParams, CgEngine::ResourceManager& resourceManager) : oceanParams(oceanParams), fastFourierTransform(resourceManager) {
        this->oceanParams.spectrumParams.alpha = calculateAlpha();
        this->oceanParams.spectrumParams.omega_p = calculateOmegaP();
        this->initialSpectrumShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/init-spectrum");
        this->timeSpectrumShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/simulate-ocean");
        this->conjugateSpectrumShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/conjugate");
        this->finalTexturesShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/compute-final");
    }

    OceanCascade::~OceanCascade() {
        delete gaussianNoise;
        delete initialSpectrum;
        delete waveData;
        delete dxDz;
        delete dyDxz;
        delete dyxDyz;
        delete dxxDzz;

        delete displacement;
        delete derivatives;
        delete turbulence;

        delete timeSpectrumDescriptorSet;
        delete finalTexturesDescriptorSet;
    }

    void OceanCascade::calculateInitialState() {
        generateGaussianNoise();
        initTextures();

        struct InitialSpectrumUBOData {
            float T;
            float gamma;
            float alpha;
            float omega_p;
            glm::vec2 wind;
            int size;
            float length;
            float depth;
            float g;
            float cutoffLow;
            float cutoffHigh;
        };

        auto* initialSpectrumUBO = CgEngine::GraphicsObjectsFactory::createUniformBuffer(sizeof(InitialSpectrumUBOData));

        CgEngine::DescriptorSetSpecification initialSpectrumDescriptorSetSpec{};
        initialSpectrumDescriptorSetSpec.layout = initialSpectrumShader->getDescriptorSetLayout();
        initialSpectrumDescriptorSetSpec.uboBindings = {
            {3, initialSpectrumUBO}
        };
        initialSpectrumDescriptorSetSpec.texture2DBindings = {
            {0, gaussianNoise}
        };
        initialSpectrumDescriptorSetSpec.attachmentImageBindings = {
            {1, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, initialSpectrum},
            {2, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, waveData},
        };
        auto* initialSpectrumDescriptorSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(initialSpectrumDescriptorSetSpec);

        InitialSpectrumUBOData initialSpectrumUboData{};
        initialSpectrumUboData.T = oceanParams.spectrumParams.T;
        initialSpectrumUboData.gamma = oceanParams.spectrumParams.gamma;
        initialSpectrumUboData.alpha = oceanParams.spectrumParams.alpha;
        initialSpectrumUboData.omega_p = oceanParams.spectrumParams.omega_p;
        initialSpectrumUboData.wind = oceanParams.spectrumParams.wind;
        initialSpectrumUboData.size = oceanParams.size;
        initialSpectrumUboData.length = oceanParams.length;
        initialSpectrumUboData.depth = oceanParams.depth;
        initialSpectrumUboData.g = oceanParams.g;
        initialSpectrumUboData.cutoffLow = oceanParams.spectrumParams.cutoffLow;
        initialSpectrumUboData.cutoffHigh = oceanParams.spectrumParams.cutoffHigh;
        initialSpectrumUBO->setData(&initialSpectrumUboData, sizeof(InitialSpectrumUBOData));

        CgEngine::Renderer::bindComputePipeline(initialSpectrumShader->getComputePipeline());
        CgEngine::Renderer::injectBarriersForDescriptorSet(initialSpectrumDescriptorSet);
        CgEngine::Renderer::bindDescriptorSet(initialSpectrumDescriptorSet, 0);
        CgEngine::Renderer::dispatchCompute(oceanParams.size / 8, oceanParams.size / 8, 1);

        CgEngine::DescriptorSetSpecification conjugateSpectrumDescriptorSetSpec{};
        conjugateSpectrumDescriptorSetSpec.layout = conjugateSpectrumShader->getDescriptorSetLayout();
        conjugateSpectrumDescriptorSetSpec.attachmentImageBindings = {
            {0, ~0u, true, CgEngine::ShaderStorageAccess::ReadWrite, initialSpectrum}
        };

        auto* conjugateSpectrumDescriptorSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(conjugateSpectrumDescriptorSetSpec);

        CgEngine::Renderer::bindComputePipeline(conjugateSpectrumShader->getComputePipeline());
        CgEngine::Renderer::injectBarriersForDescriptorSet(conjugateSpectrumDescriptorSet);
        CgEngine::Renderer::bindDescriptorSet(conjugateSpectrumDescriptorSet, 0);
        CgEngine::Renderer::dispatchCompute(oceanParams.size / 8, oceanParams.size / 8, 1);

        delete initialSpectrumUBO;
        delete initialSpectrumDescriptorSet;
        delete conjugateSpectrumDescriptorSet;

        CgEngine::DescriptorSetSpecification timeSpectrumDescriptorSetSpec{};
        timeSpectrumDescriptorSetSpec.layout = timeSpectrumShader->getDescriptorSetLayout();
        timeSpectrumDescriptorSetSpec.attachmentImageBindings = {
            {0, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, initialSpectrum},
            {1, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, waveData},
            {2, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, dxDz},
            {3, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, dyDxz},
            {4, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, dyxDyz},
            {5, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, dxxDzz},
        };

        timeSpectrumDescriptorSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(timeSpectrumDescriptorSetSpec);

        CgEngine::DescriptorSetSpecification finalTexturesDescriptorSetSpec{};
        finalTexturesDescriptorSetSpec.layout = finalTexturesShader->getDescriptorSetLayout();
        finalTexturesDescriptorSetSpec.attachmentImageBindings = {
            {0, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, dxDz},
            {1, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, dyDxz},
            {2, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, dyxDyz},
            {3, ~0u, true, CgEngine::ShaderStorageAccess::ReadOnly, dxxDzz},
            {4, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, displacement},
            {5, ~0u, true, CgEngine::ShaderStorageAccess::WriteOnly, derivatives},
            {6, ~0u, true, CgEngine::ShaderStorageAccess::ReadWrite, turbulence},
        };
        finalTexturesDescriptorSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(finalTexturesDescriptorSetSpec);
    }

    void OceanCascade::calculateStateAtTime(float time, float deltaT) {
        CgEngine::Renderer::bindComputePipeline(timeSpectrumShader->getComputePipeline());
        CgEngine::Renderer::injectBarriersForDescriptorSet(timeSpectrumDescriptorSet);
        CgEngine::Renderer::bindDescriptorSet(timeSpectrumDescriptorSet, 0);

        SimulateOceanPC simulateOceanPc{};
        simulateOceanPc.time = time;
        CgEngine::Renderer::setPushConstants(&simulateOceanPc, sizeof(SimulateOceanPC));
        CgEngine::Renderer::dispatchCompute(oceanParams.size / 8, oceanParams.size / 8, 1);

        fastFourierTransform.inverseTransform(dxDz);
        fastFourierTransform.inverseTransform(dyDxz);
        fastFourierTransform.inverseTransform(dyxDyz);
        fastFourierTransform.inverseTransform(dxxDzz);

        CgEngine::Renderer::bindComputePipeline(finalTexturesShader->getComputePipeline());
        CgEngine::Renderer::injectBarriersForDescriptorSet(finalTexturesDescriptorSet);
        CgEngine::Renderer::bindDescriptorSet(finalTexturesDescriptorSet, 0);

        FinalTexturesPC finalTexturesPc{};
        finalTexturesPc.lambda = 1.0f;
        finalTexturesPc.deltaTime = deltaT;
        CgEngine::Renderer::setPushConstants(&finalTexturesPc, sizeof(FinalTexturesPC));
        CgEngine::Renderer::dispatchCompute(oceanParams.size / 8, oceanParams.size / 8, 1);
    }

    void OceanCascade::generateGaussianNoise() {
        delete gaussianNoise;
        auto* gaussianNoiseData = new glm::vec2[oceanParams.size * oceanParams.size];
        double twoPi = glm::pi<double>() * 2;

        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(0, 1);

        for (int i = 0; i < oceanParams.size * oceanParams.size; i++) {
            double U = dist(mt);
            double V = dist(mt);

            double _tmp = sqrt(-2 * log(U));
            double twoPiV = twoPi * V;
            gaussianNoiseData[i].r = (float)(_tmp * cos(twoPiV));
            gaussianNoiseData[i].g = (float)(_tmp * sin(twoPiV));
        }
        gaussianNoise = CgEngine::GraphicsObjectsFactory::createTexture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                gaussianNoiseData,
                CgEngine::MipMapFiltering::Nearest
        );
        delete[] gaussianNoiseData;
    }

    void OceanCascade::initTextures() {
        CgEngine::AttachmentSpecification rgbaAttachmentSpec{};
        rgbaAttachmentSpec.type = CgEngine::AttachmentType::RGBA32F;
        rgbaAttachmentSpec.width = oceanParams.size;
        rgbaAttachmentSpec.height = oceanParams.size;
        rgbaAttachmentSpec.usableAsTexture = true;
        rgbaAttachmentSpec.usableAsStorageImage = true;
        rgbaAttachmentSpec.textureWrap = CgEngine::TextureWrap::Repeat;
        rgbaAttachmentSpec.mipMapFiltering = CgEngine::MipMapFiltering::Nearest;

        initialSpectrum = CgEngine::GraphicsObjectsFactory::createAttachment(rgbaAttachmentSpec);
        waveData = CgEngine::GraphicsObjectsFactory::createAttachment(rgbaAttachmentSpec);

        CgEngine::AttachmentSpecification rgAttachmentSpec{};
        rgAttachmentSpec.type = CgEngine::AttachmentType::RG32F;
        rgAttachmentSpec.width = oceanParams.size;
        rgAttachmentSpec.height = oceanParams.size;
        rgAttachmentSpec.usableAsTexture = true;
        rgAttachmentSpec.usableAsStorageImage = true;
        rgAttachmentSpec.textureWrap = CgEngine::TextureWrap::Repeat;
        rgAttachmentSpec.mipMapFiltering = CgEngine::MipMapFiltering::Nearest;

        dxDz = CgEngine::GraphicsObjectsFactory::createAttachment(rgAttachmentSpec);
        dyDxz = CgEngine::GraphicsObjectsFactory::createAttachment(rgAttachmentSpec);
        dyxDyz = CgEngine::GraphicsObjectsFactory::createAttachment(rgAttachmentSpec);
        dxxDzz =CgEngine::GraphicsObjectsFactory::createAttachment(rgAttachmentSpec);

        CgEngine::AttachmentSpecification rgbaBilinearAttachmentSpec{};
        rgbaBilinearAttachmentSpec.type = CgEngine::AttachmentType::RGBA32F;
        rgbaBilinearAttachmentSpec.width = oceanParams.size;
        rgbaBilinearAttachmentSpec.height = oceanParams.size;
        rgbaBilinearAttachmentSpec.usableAsTexture = true;
        rgbaBilinearAttachmentSpec.usableAsStorageImage = true;
        rgbaBilinearAttachmentSpec.textureWrap = CgEngine::TextureWrap::Repeat;
        rgbaBilinearAttachmentSpec.mipMapFiltering = CgEngine::MipMapFiltering::Bilinear;

        displacement = CgEngine::GraphicsObjectsFactory::createAttachment(rgbaBilinearAttachmentSpec);
        derivatives = CgEngine::GraphicsObjectsFactory::createAttachment(rgbaBilinearAttachmentSpec);

        std::vector<float> zeroData(oceanParams.size * oceanParams.size * 4, 0.0);
        turbulence = CgEngine::GraphicsObjectsFactory::createAttachment(rgbaBilinearAttachmentSpec);
    }

    float OceanCascade::calculateAlpha() const {
        float U_10 = 0.5;
        float F = 100000.0;
        return 0.076 * pow(U_10 * U_10 / F / oceanParams.g, 0.22);
    }

    float OceanCascade::calculateOmegaP() const {
        float U_10 = 0.5;
        float F = 100000.0;
        return 22.0 * pow(oceanParams.g * oceanParams.g / U_10 / F, 1.0 / 3.0);
    }
}
