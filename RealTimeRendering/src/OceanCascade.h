#pragma once

#include <Rendering/Attachment.h>
#include <Rendering/Texture2D.h>
#include <Rendering/CustomPipeline.h>
#include <Rendering/PushConstants.h>
#include <Resources/ResRef.h>
#include <Resources/ResourceManager.h>
#include "FastFourierTransform.h"

namespace RTR {
    struct SpectrumParams {
        float T = 20.0; // Base period
        float gamma = 3.3; // gamma of JONSWAP
        float alpha; // alpha of JONSWAP
        float omega_p; // omega_p of JONSWAP
        float cutoffLow = 0.0001;
        float cutoffHigh = 2 * 6 * glm::pi<float>() / 17;
        glm::vec2 wind = {10, 5}; // Wind direction, the norm defines the strength
    };

    struct OceanParams {
        int size = 256; // Has to be a power of two, resolution of the simulation
        float length = 250; // Size of the ocean in meters
        float depth = 500; // The depth in meters
        float g = 9.81; // Gravity constant
        SpectrumParams spectrumParams;
    };

    class OceanCascade {
    public:
        explicit OceanCascade(const OceanParams& oceanParams, CgEngine::ResourceManager& resourceManager);

        ~OceanCascade();

        void calculateInitialState();

        void calculateStateAtTime(float time, float deltaT);

    private:
        void generateGaussianNoise();

        void initTextures();

        float calculateAlpha() const;

        float calculateOmegaP() const;

        struct SimulateOceanPC {
            float time;
        };
        CgEngine::DescriptorSet* timeSpectrumDescriptorSet = nullptr;
        CgEngine::PushConstants* timeSpectrumPushConstants = nullptr;

        struct FinalTexturesPC {
            float deltaTime;
            float lambda;
        };
        CgEngine::DescriptorSet* finalTexturesDescriptorSet = nullptr;
        CgEngine::PushConstants* finalTexturesPushConstants = nullptr;

    public:
        OceanParams oceanParams;

        CgEngine::Texture2D* gaussianNoise = nullptr;
        CgEngine::Attachment* initialSpectrum = nullptr;
        CgEngine::Attachment* waveData = nullptr;
        CgEngine::Attachment* dxDz = nullptr;
        CgEngine::Attachment* dyDxz = nullptr;
        CgEngine::Attachment* dyxDyz = nullptr;
        CgEngine::Attachment* dxxDzz = nullptr;

        CgEngine::Attachment* displacement = nullptr;
        CgEngine::Attachment* derivatives = nullptr;
        CgEngine::Attachment* turbulence = nullptr;

        CgEngine::ResRef<CgEngine::CustomComputePipeline> initialSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> conjugateSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> timeSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> finalTexturesShader;

        FastFourierTransform fastFourierTransform;
    };
}
