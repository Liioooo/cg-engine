#include <Rendering/Shader.h>
#include <Logging.h>
#include <Rendering/UniformBuffer.h>
#include "OceanCascade.h"

namespace RTR {
    OceanCascade::OceanCascade(const OceanParams& oceanParams, CgEngine::ResourceManager& resourceManager)
            : oceanParams(oceanParams), fastFourierTransform(resourceManager) {
        this->oceanParams.spectrumParams.alpha = calculateAlpha();
        this->oceanParams.spectrumParams.omega_p = calculateOmegaP();
        this->initialSpectrumShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/init-spectrum");
        this->timeSpectrumShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/simulate-ocean");
        this->conjugateSpectrumShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/conjugate");
        this->finalTexturesShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/compute-final");
    }

    OceanCascade::~OceanCascade()
    {
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
    }

    void OceanCascade::calculateInitialState() {
        generateGaussianNoise();
        initTextures();
        initialSpectrumShader->bind();
        // initialSpectrumShader->setFloat("u_T", oceanParams.spectrumParams.T);
        initialSpectrumShader->setFloat("u_gamma", oceanParams.spectrumParams.gamma);
        initialSpectrumShader->setFloat("u_alpha", oceanParams.spectrumParams.alpha);
        initialSpectrumShader->setFloat("u_omega_p", oceanParams.spectrumParams.omega_p);
        // initialSpectrumShader->setVec2("u_wind", oceanParams.spectrumParams.wind);
        initialSpectrumShader->setInt("u_size", oceanParams.size);
        initialSpectrumShader->setFloat("u_length", oceanParams.length);
        initialSpectrumShader->setFloat("u_depth", oceanParams.depth);
        initialSpectrumShader->setFloat("u_g", oceanParams.g);
        initialSpectrumShader->setFloat("u_cutoffLow", oceanParams.spectrumParams.cutoffLow);
        initialSpectrumShader->setFloat("u_cutoffHigh", oceanParams.spectrumParams.cutoffHigh);
        initialSpectrumShader->setImage2D(*gaussianNoise, 0, CgEngine::ShaderStorageAccess::ReadOnly, 0);
        initialSpectrumShader->setImage2D(*initialSpectrum, 1, CgEngine::ShaderStorageAccess::WriteOnly, 0);
        initialSpectrumShader->setImage2D(*waveData, 2, CgEngine::ShaderStorageAccess::WriteOnly, 0);
        initialSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        initialSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
        conjugateSpectrumShader->bind();
        conjugateSpectrumShader->setImage2D(*initialSpectrum, 0, CgEngine::ShaderStorageAccess::ReadWrite, 0);
        conjugateSpectrumShader->setInt("u_size", oceanParams.size);
        conjugateSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        conjugateSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
    }

    void OceanCascade::calculateStateAtTime(float time, float deltaT) {
        timeSpectrumShader->bind();
        timeSpectrumShader->setFloat("u_time", time);
        timeSpectrumShader->setImage2D(*initialSpectrum, 0, CgEngine::ShaderStorageAccess::ReadOnly);
        timeSpectrumShader->setImage2D(*waveData, 1, CgEngine::ShaderStorageAccess::ReadOnly);
        timeSpectrumShader->setImage2D(*dxDz, 2, CgEngine::ShaderStorageAccess::WriteOnly);
        timeSpectrumShader->setImage2D(*dyDxz, 3, CgEngine::ShaderStorageAccess::WriteOnly);
        timeSpectrumShader->setImage2D(*dyxDyz, 4, CgEngine::ShaderStorageAccess::WriteOnly);
        timeSpectrumShader->setImage2D(*dxxDzz, 5, CgEngine::ShaderStorageAccess::WriteOnly);
        timeSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        timeSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});

        fastFourierTransform.inverseTransform(*dxDz);
        fastFourierTransform.inverseTransform(*dyDxz);
        fastFourierTransform.inverseTransform(*dyxDyz);
        fastFourierTransform.inverseTransform(*dxxDzz);

        finalTexturesShader->bind();
        finalTexturesShader->setFloat("u_lambda", 1);
        finalTexturesShader->setFloat("u_deltaTime", deltaT);
        finalTexturesShader->setImage2D(*dxDz, 0, CgEngine::ShaderStorageAccess::ReadOnly);
        finalTexturesShader->setImage2D(*dyDxz, 1, CgEngine::ShaderStorageAccess::ReadOnly);
        finalTexturesShader->setImage2D(*dyxDyz, 2, CgEngine::ShaderStorageAccess::ReadOnly);
        finalTexturesShader->setImage2D(*dxxDzz, 3, CgEngine::ShaderStorageAccess::ReadOnly);
        finalTexturesShader->setImage2D(*displacement, 4, CgEngine::ShaderStorageAccess::WriteOnly);
        finalTexturesShader->setImage2D(*derivatives, 5, CgEngine::ShaderStorageAccess::WriteOnly);
        finalTexturesShader->setImage2D(*turbulence, 6, CgEngine::ShaderStorageAccess::ReadWrite);
        finalTexturesShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        finalTexturesShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});

        displacement->generateMipMaps();
        derivatives->generateMipMaps();
        turbulence->generateMipMaps();
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
        gaussianNoise = new CgEngine::Texture2D(
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
        initialSpectrum = new CgEngine::Texture2D(
                CgEngine::TextureFormat::Float32A,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        waveData = new CgEngine::Texture2D(
                CgEngine::TextureFormat::Float32A,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        dxDz = new CgEngine::Texture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        dyDxz = new CgEngine::Texture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        dyxDyz = new CgEngine::Texture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        dxxDzz = new CgEngine::Texture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Nearest
        );
        displacement = new CgEngine::Texture2D(
                CgEngine::TextureFormat::Float32A,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Trilinear
        );
        derivatives = new CgEngine::Texture2D(
                CgEngine::TextureFormat::Float32A,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                CgEngine::MipMapFiltering::Trilinear
        );
        std::vector<float> zeroData(oceanParams.size * oceanParams.size * 4, 0.0);
        turbulence = new CgEngine::Texture2D(
                CgEngine::TextureFormat::Float32A,
                oceanParams.size,
                oceanParams.size,
                CgEngine::TextureWrap::Repeat,
                &zeroData[0],
                CgEngine::MipMapFiltering::Trilinear
        );
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