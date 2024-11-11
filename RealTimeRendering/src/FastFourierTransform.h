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
        CgEngine::CustomComputeShader* precomputeTwiddleFactorsAndInputIndicesShader;
        CgEngine::CustomComputeShader* permuteShader;
        CgEngine::CustomComputeShader* horizontalStepInverseFftShader;
        CgEngine::CustomComputeShader* verticalStepInverseFftShader;

        CgEngine::Texture2D* buffer;
        CgEngine::Texture2D* twiddleFactors;
    };
}
