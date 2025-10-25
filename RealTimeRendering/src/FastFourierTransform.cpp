#include "FastFourierTransform.h"
#include "CgEngine/Rendering/GraphicsObjectsFactory.h"
#include "CgEngine/Rendering/Renderer.h"

namespace RTR {

    FastFourierTransform::FastFourierTransform(CgEngine::ResourceManager& resourceManager): resourceManager(resourceManager) {
        precomputeTwiddleFactorsAndInputIndicesShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/precomputeTwiddleFactorsAndInputIndices");
        permuteShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/permute");
        horizontalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/horizontalStepInverseFft");
        verticalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputePipeline>("ocean/fft/verticalStepInverseFft");

        pushConstants = CgEngine::GraphicsObjectsFactory::createPushConstants("pc_fft");
        pushConstants->init<PCFft>();
        pushConstants->mapUniform(&PCFft::step, "step");
        pushConstants->mapUniform(&PCFft::pingPong, "pingPong");

    }

    FastFourierTransform::~FastFourierTransform() {
        delete buffer;
        delete twiddleFactors;
    }

    void FastFourierTransform::inverseTransform(CgEngine::Attachment* input, bool outputToInput, bool permute) {
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


            auto* pc = CgEngine::GraphicsObjectsFactory::createPushConstants("pc_precompute");
            pc->init<PushConstantData>();
            pc->mapUniform(&PushConstantData::size, "size");
            pc->setData(&pcData, sizeof(int));

            CgEngine::Renderer::bindComputePipeline(precomputeTwiddleFactorsAndInputIndicesShader->getComputePipeline());
            CgEngine::Renderer::bindDescriptorSet(descSet, 0);
            CgEngine::Renderer::setPushConstants({pc}, 1);
            CgEngine::Renderer::dispatchCompute(logSize, static_cast<int>(input->getHeight() / 2.0 / 8.0), 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            delete descSet;
            delete pc;
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

        CgEngine::DescriptorSetSpecification fftDescSetSpec{};
        fftDescSetSpec.layout = horizontalStepInverseFftShader->getDescriptorSetLayout();
        fftDescSetSpec.attachmentImageBindings = {
            {0, ~0u, true, CgEngine::ShaderImageAccess::ReadOnly, twiddleFactors},
            {1, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, input},
            {2, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, buffer},
        };
        auto* fftDescSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(fftDescSetSpec);

        CgEngine::Renderer::bindComputePipeline(horizontalStepInverseFftShader->getComputePipeline());
        CgEngine::Renderer::bindDescriptorSet(fftDescSet, 0);

        PCFft pcData{};

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;

            pcData.step = i;
            pcData.pingPong = pingPong;
            pushConstants->setData(&pcData, sizeof(PCFft));

            CgEngine::Renderer::setPushConstants({pushConstants}, 1);
            CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }

        CgEngine::Renderer::bindComputePipeline(verticalStepInverseFftShader->getComputePipeline());
        CgEngine::Renderer::bindDescriptorSet(fftDescSet, 0);

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;

            pcData.step = i;
            pcData.pingPong = pingPong;
            pushConstants->setData(&pcData, sizeof(PCFft));

            CgEngine::Renderer::setPushConstants({pushConstants}, 1);
            CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }

//    if (pingPong && outputToInput) {
//        Graphics.Blit(buffer, input);
//    }
//
//    if (!pingPong && !outputToInput) {
//        TODO: Implement this
//        Graphics.Blit(input, buffer);
//    }

        if (permute) {
            CgEngine::Renderer::bindComputePipeline(permuteShader->getComputePipeline());

            CgEngine::DescriptorSetSpecification permuteDescSetSpec{};
            permuteDescSetSpec.layout = permuteShader->getDescriptorSetLayout();

            if (outputToInput) {
                permuteDescSetSpec.attachmentImageBindings = {
                    {0, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, input},
                };
            } else {
                permuteDescSetSpec.attachmentImageBindings = {
                        {0, ~0u, true, CgEngine::ShaderImageAccess::ReadWrite, buffer},
                };
            }
            auto* permuteDescSet = CgEngine::GraphicsObjectsFactory::createDescriptorSet(permuteDescSetSpec);
            CgEngine::Renderer::bindDescriptorSet(permuteDescSet, 0);
            CgEngine::Renderer::dispatchCompute(input->getWidth() / 8, input->getHeight() / 8, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            delete permuteDescSet;
        }
        /*
        if (scale) {
            fftShader.SetInt(PROP_ID_SIZE, size);
            fftShader.SetTexture(KERNEL_SCALE, PROP_ID_BUFFER0, outputToInput ? input : buffer);
            fftShader.Dispatch(KERNEL_SCALE, size / LOCAL_WORK_GROUPS_X, size / LOCAL_WORK_GROUPS_Y, 1);
        }
        */
    }

}
