#include <Rendering/Shader.h>
#include <Logging.h>
#include "OceanCascade.h"

namespace RTR {
    OceanCascade::OceanCascade(const OceanParams& oceanParams, CgEngine::CustomComputeShader* initialSpectrumShader,
                               CgEngine::CustomComputeShader* conjugateSpectrumShader,
                               CgEngine::CustomComputeShader* timeSpectrumShader) :
            oceanParams(oceanParams), initialSpectrumShader(initialSpectrumShader),
            conjugateSpectrumShader(conjugateSpectrumShader), fastFourierTransform(), timeSpectrumShader(timeSpectrumShader) {}

    void OceanCascade::calculateInitialState() {
        generateGaussianNoise();
        initTextures();
        initialSpectrumShader->bind();
        initialSpectrumShader->setImage2D(*gaussianNoise, 0, CgEngine::ShaderStorageAccess::ReadWrite, 0);
        initialSpectrumShader->setImage2D(*initialSpectrum, 1, CgEngine::ShaderStorageAccess::ReadWrite, 0);
        initialSpectrumShader->setImage2D(*waveData, 2, CgEngine::ShaderStorageAccess::ReadWrite, 0);
        initialSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        initialSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::All});
        conjugateSpectrumShader->bind();
        conjugateSpectrumShader->setImage2D(*initialSpectrum, 0, CgEngine::ShaderStorageAccess::ReadWrite, 0);
        conjugateSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        conjugateSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::All});
    }

    void OceanCascade::calculateStateAtTime(float time) {
        timeSpectrumShader->bind();
        timeSpectrumShader->setFloat("time", time);
        timeSpectrumShader->setImage2D(*initialSpectrum, 0, CgEngine::ShaderStorageAccess::ReadOnly);
        timeSpectrumShader->setImage2D(*waveData, 1, CgEngine::ShaderStorageAccess::ReadOnly);
        timeSpectrumShader->setImage2D(*timeSpectrum, 2, CgEngine::ShaderStorageAccess::ReadWrite);
        timeSpectrumShader->dispatch(oceanParams.size / 8, oceanParams.size / 8, 1);
        timeSpectrumShader->waitForMemoryBarrier({CgEngine::MemoryBarrierBit::All});

//        fft(dxTexture, precomputeBuffer, outputFft, N);
        fastFourierTransform.inverseTransform(*timeSpectrum);
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
        timeSpectrum = new CgEngine::Texture2D(
                CgEngine::TextureFormat::RedGreenFloat32,
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
    }
}