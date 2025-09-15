#pragma once

#include <Rendering/TextureCube.h>
#include <Rendering/CustomShaders.h>
#include <Resources/ResRef.h>
#include <Resources/ResourceManager.h>

namespace RTR {
    class FastFourierTransform {
    public:
        FastFourierTransform(CgEngine::ResourceManager& resourceManager);
        ~FastFourierTransform();
        void inverseTransform(CgEngine::Texture2D& input, bool outputToInput = true, bool permute = true);
    private:
        CgEngine::ResourceManager& resourceManager;
        CgEngine::ResRef<CgEngine::CustomComputeShader> precomputeTwiddleFactorsAndInputIndicesShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputeShader> permuteShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputeShader> horizontalStepInverseFftShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputeShader> verticalStepInverseFftShader = nullptr;

        CgEngine::Texture2D* buffer = nullptr;
        CgEngine::Texture2D* twiddleFactors = nullptr;
    };
}
