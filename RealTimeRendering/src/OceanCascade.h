#pragma once

#include <Rendering/Texture.h>
#include <Rendering/CustomShaders.h>
#include <Resources/ResRef.h>
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
        explicit OceanCascade(const OceanParams& oceanParams,
                              CgEngine::ResourceManager& resourceManager);

        void calculateInitialState();

        void calculateStateAtTime(float time, float deltaT);

    private:
        void generateGaussianNoise();

        void initTextures();

        float calculateAlpha() const;

        float calculateOmegaP() const;
    public:
        OceanParams oceanParams;

        CgEngine::Texture2D* gaussianNoise = nullptr;
        CgEngine::Texture2D* initialSpectrum = nullptr;
        CgEngine::Texture2D* waveData = nullptr;
        CgEngine::Texture2D* dxDz = nullptr;
        CgEngine::Texture2D* dyDxz = nullptr;
        CgEngine::Texture2D* dyxDyz = nullptr;
        CgEngine::Texture2D* dxxDzz = nullptr;

        CgEngine::Texture2D* displacement = nullptr;
        CgEngine::Texture2D* derivatives = nullptr;
        CgEngine::Texture2D* turbulence = nullptr;

        CgEngine::ResRef<CgEngine::CustomComputeShader> initialSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputeShader> conjugateSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputeShader> timeSpectrumShader;
        CgEngine::ResRef<CgEngine::CustomComputeShader> finalTexturesShader;

        FastFourierTransform fastFourierTransform;
    };
}
