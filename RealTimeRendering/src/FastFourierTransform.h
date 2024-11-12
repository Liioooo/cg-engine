#pragma once

#include <Rendering/Texture.h>
#include <Rendering/CustomShaders.h>

namespace RTR {
    class FastFourierTransform {
    public:
        FastFourierTransform();
        ~FastFourierTransform();
        void inverseTransform(CgEngine::Texture2D& input, bool outputToInput = true, bool permute = true);
    private:
        CgEngine::CustomComputeShader* precomputeTwiddleFactorsAndInputIndicesShader = nullptr;
        CgEngine::CustomComputeShader* permuteShader = nullptr;
        CgEngine::CustomComputeShader* horizontalStepInverseFftShader = nullptr;
        CgEngine::CustomComputeShader* verticalStepInverseFftShader = nullptr;

        CgEngine::Texture2D* buffer = nullptr;
        CgEngine::Texture2D* twiddleFactors = nullptr;
    };
}
