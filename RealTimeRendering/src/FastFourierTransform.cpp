#include "FastFourierTransform.h"

namespace RTR {

    FastFourierTransform::FastFourierTransform(CgEngine::ResourceManager& resourceManager): resourceManager(resourceManager) {
        precomputeTwiddleFactorsAndInputIndicesShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/fft/precomputeTwiddleFactorsAndInputIndices");
        permuteShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/fft/permute");
        horizontalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/fft/horizontalStepInverseFft");
        verticalStepInverseFftShader = resourceManager.getResource<CgEngine::CustomComputeShader>("ocean/fft/verticalStepInverseFft");
    }

    FastFourierTransform::~FastFourierTransform() {
        delete buffer;
        delete twiddleFactors;
    }

    void FastFourierTransform::inverseTransform(CgEngine::Texture2D& input, bool outputToInput, bool permute) {
        int logSize = (int)log2(input.getWidth());
        bool pingPong = false;

        if (twiddleFactors == nullptr || twiddleFactors->getWidth() != logSize) {
            twiddleFactors = new CgEngine::Texture2D(
                    CgEngine::TextureFormat::Float32A,
                    logSize,
                    input.getHeight(),
                    CgEngine::TextureWrap::Repeat,
                    CgEngine::MipMapFiltering::Nearest
            );
            precomputeTwiddleFactorsAndInputIndicesShader->bind();
            precomputeTwiddleFactorsAndInputIndicesShader->setInt("u_size", (int)input.getWidth());
            precomputeTwiddleFactorsAndInputIndicesShader->setImage2D(*twiddleFactors, 0, CgEngine::ShaderStorageAccess::ReadWrite);
            precomputeTwiddleFactorsAndInputIndicesShader->dispatch(logSize, static_cast<int>(input.getHeight() / 2.0 / 8.0), 1);
            precomputeTwiddleFactorsAndInputIndicesShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
        }
        if (buffer == nullptr || buffer->getWidth() != input.getWidth()) {
            buffer = new CgEngine::Texture2D(
                    CgEngine::TextureFormat::RedGreenFloat32,
                    input.getWidth(),
                    input.getHeight(),
                    CgEngine::TextureWrap::Repeat,
                    CgEngine::MipMapFiltering::Nearest
            );
        }

        horizontalStepInverseFftShader->bind();
        horizontalStepInverseFftShader->setImage2D(*twiddleFactors, 0, CgEngine::ShaderStorageAccess::ReadOnly);
        horizontalStepInverseFftShader->setImage2D(input, 1, CgEngine::ShaderStorageAccess::ReadWrite);
        horizontalStepInverseFftShader->setImage2D(*buffer, 2, CgEngine::ShaderStorageAccess::ReadWrite);

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;
            horizontalStepInverseFftShader->setInt("u_step", i);
            horizontalStepInverseFftShader->setBool("u_pingPong", pingPong);
            horizontalStepInverseFftShader->dispatch(input.getWidth() / 8, input.getHeight() / 8, 1);
            horizontalStepInverseFftShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
        }

        verticalStepInverseFftShader->bind();
        verticalStepInverseFftShader->setImage2D(*twiddleFactors, 0, CgEngine::ShaderStorageAccess::ReadOnly);
        verticalStepInverseFftShader->setImage2D(input, 1, CgEngine::ShaderStorageAccess::ReadWrite);
        verticalStepInverseFftShader->setImage2D(*buffer, 2, CgEngine::ShaderStorageAccess::ReadWrite);

        for (int i = 0; i < logSize; i++) {
            pingPong = !pingPong;
            verticalStepInverseFftShader->setInt("u_step", i);
            verticalStepInverseFftShader->setBool("u_pingPong", pingPong);
            verticalStepInverseFftShader->dispatch(input.getWidth() / 8, input.getHeight() / 8, 1);
            verticalStepInverseFftShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
        }

//    if (pingPong && outputToInput) {
//        Graphics.Blit(buffer, input);
//    }
//
//    if (!pingPong && !outputToInput) {
//        TODO: Implement this
//        Graphics.Blit(input, buffer);
//    }

        permuteShader->bind();
        if (permute) {
            if (outputToInput) {
                permuteShader->setImage2D(input, 0, CgEngine::ShaderStorageAccess::ReadWrite);
            } else {
                permuteShader->setImage2D(*buffer, 0, CgEngine::ShaderStorageAccess::ReadWrite);
            }
            permuteShader->dispatch(input.getWidth() / 8, input.getHeight() / 8, 1);
            permuteShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::ShaderImageAccess});
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
