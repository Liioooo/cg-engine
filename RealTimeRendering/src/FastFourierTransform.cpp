#include "FastFourierTransform.h"
#include "CgEngine/Rendering/GraphicsObjectsFactory.h"
#include "CgEngine/Rendering/Renderer.h"

namespace RTR {

    FastFourierTransform::FastFourierTransform(CgEngine::ResourceManager& resourceManager): resourceManager(resourceManager) {
        precomputeTwiddleFactorsAndInputIndicesShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/precomputeTwiddleFactorsAndInputIndices");
        permuteShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/permute");
        horizontalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/horizontalStepInverseFft");
        verticalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/verticalStepInverseFft");
    }

    FastFourierTransform::~FastFourierTransform() {
        delete buffer;
        delete twiddleFactors;
    }

    void FastFourierTransform::inverseTransform(CgEngine::Attachment* input) {
        int logSize = (int)log2(input->getWidth());
        bool pingPong = false;

        if (twiddleFactors == nullptr || twiddleFactors->getWidth() != logSize) {
            delete twiddleFactors;

            CgEngine::AttachmentSpecification spec{};
            spec.type = CgEngine::AttachmentType::RGBA32F;
            spec.width = logSize;
            spec.height = input->getHeight();
            spec.usableAsTexture = true;
            spec.textureWrap = CgEngine::TextureWrap::Repeat;
            spec.mipMapFiltering = CgEngine::MipMapFiltering::Nearest;
            twiddleFactors = CgEngine::GraphicsObjectsFactory::createAttachment(spec);

            CgEngine::DescriptorSetSpecification descSetSpec{};
            descSetSpec.layout = precomputeTwiddleFactorsAndInputIndicesShader->getDescriptorSetLayout();
            descSetSpec.attachmentImageBindings = {
                    {0, ~0u, true, CgEngine::ShaderImageAccess::WriteOnly, twiddleFactors},
            };
            auto* descSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(descSetSpec);

            struct PushConstantData {
                int size;
            };
            PushConstantData pcData{};
            pcData.size = input->getWidth();

            CgEngine::Renderer::bindComputePipeline(precomputeTwiddleFactorsAndInputIndicesShader->getComputePipeline());
            CgEngine::Renderer::bindDescriptorSet(descSet, 0);
            CgEngine::Renderer::setPushConstants(&pcData, sizeof(int));
            CgEngine::Renderer::dispatchCompute(logSize, static_cast<int>(input->getHeight() / 2.0 / 8.0), 1);
            CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(twiddleFactors);

            delete descSet;
        }
        if (buffer == nullptr || buffer->getWidth() != input->getWidth()) {
            delete buffer;

            CgEngine::AttachmentSpecification spec{};
            spec.type = CgEngine::AttachmentType::RG32F;
            spec.width = input->getWidth();
            spec.height = input->getHeight();
            spec.usableAsTexture = true;
            spec.textureWrap = CgEngine::TextureWrap::Repeat;
            spec.mipMapFiltering = CgEngine::MipMapFiltering::Nearest;

            buffer = CgEngine::GraphicsObjectsFactory::createAttachment(spec);
        }

        if (fftDescriptorSets.find(input) == fftDescriptorSets.end()) {
            CgEngine::DescriptorSetSpecification fftDescSetSpec{};
            fftDescSetSpec.layout = horizontalStepInverseFftShader->getDescriptorSetLayout();
            fftDescSetSpec.attachmentImageBindings = {
                    {0, ~0u, true, CgEngine::ShaderImageAccess::ReadOnly, twiddleFactors},
                    {1, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, input},
                    {2, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, buffer},
            };
            auto* fftDescSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(fftDescSetSpec);
            fftDescriptorSets[input] = std::unique_ptr<CgEngine::DescriptorSet>(fftDescSet);
        }

        CgEngine::Renderer::bindComputePipeline(horizontalStepInverseFftShader->getComputePipeline());
        CgEngine::Renderer::bindDescriptorSet(fftDescriptorSets.at(input).get(), 0);

        PCFft pcData{};

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;

            pcData.step = i;
            pcData.pingPong = pingPong ? 1 : 0;

            CgEngine::Renderer::setPushConstants(&pcData, sizeof(PCFft));
            CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
            CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(input);
            CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(buffer);
        }

        CgEngine::Renderer::bindComputePipeline(verticalStepInverseFftShader->getComputePipeline());
        CgEngine::Renderer::bindDescriptorSet(fftDescriptorSets.at(input).get(), 0);

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;

            pcData.step = i;
            pcData.pingPong = pingPong ? 1 : 0;

            CgEngine::Renderer::setPushConstants(&pcData, sizeof(PCFft));
            CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
            CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(input);
            CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(buffer);
        }

        if (permuteDescriptorSets.find(input) == permuteDescriptorSets.end()) {
            CgEngine::DescriptorSetSpecification permuteDescSetSpec{};
            permuteDescSetSpec.layout = permuteShader->getDescriptorSetLayout();
            permuteDescSetSpec.attachmentImageBindings = {
                    {0, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, input},
            };
            auto* permuteDescSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(permuteDescSetSpec);
            permuteDescriptorSets[input] = std::unique_ptr<CgEngine::DescriptorSet>(permuteDescSet);
        }

        CgEngine::Renderer::bindComputePipeline(permuteShader->getComputePipeline());
        CgEngine::Renderer::bindDescriptorSet(permuteDescriptorSets.at(input).get(), 0);
        CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
        CgEngine::Renderer::memoryBarrierForAttachmentAfterComputeToCompute(input);
    }

}
