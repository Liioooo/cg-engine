#pragma once

#include <Rendering/Texture.h>
#include <Rendering/CustomShaders.h>
#include "FastFourierTransform.h"

namespace RTR {
    struct SpectrumParams {
        glm::vec2 wind = {10, 5}; // Wind direction, the norm defines the strength
        float T = 20.0; // Base period
    };

    struct OceanParams {
        int size = 256; // Has to be a power of two, resolution of the simulation
        int length = 250; // Size of the ocean in meters
        float depth = 500; // The depth in meters
        float g = 9.81; // Gravity constant
        SpectrumParams spectrumParams;
    };

    class OceanCascade {
    public:
        explicit OceanCascade(const OceanParams& oceanParams, CgEngine::CustomComputeShader* initialSpectrumShader, CgEngine::CustomComputeShader* conjugateSpectrumShader, CgEngine::CustomComputeShader* timeSpectrumShader);

        void calculateInitialState();
        void calculateStateAtTime(float time);

    private:
        void generateGaussianNoise();

        void initTextures();
    public:
        OceanParams oceanParams;

        CgEngine::Texture2D* gaussianNoise;
        CgEngine::Texture2D* initialSpectrum;
        CgEngine::Texture2D* timeSpectrum;
        CgEngine::Texture2D* waveData;

        CgEngine::CustomComputeShader* initialSpectrumShader;
        CgEngine::CustomComputeShader* conjugateSpectrumShader;
        CgEngine::CustomComputeShader* timeSpectrumShader;

        FastFourierTransform fastFourierTransform;
    };
}
