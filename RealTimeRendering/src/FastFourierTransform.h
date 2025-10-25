#pragma once

#include <Rendering/CustomPipeline.h>
#include <Resources/ResRef.h>
#include <Resources/ResourceManager.h>

namespace RTR {
    class FastFourierTransform {
    public:
        FastFourierTransform(CgEngine::ResourceManager& resourceManager);
        ~FastFourierTransform();
        void inverseTransform(CgEngine::Attachment* input);
    private:
        CgEngine::ResourceManager& resourceManager;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> precomputeTwiddleFactorsAndInputIndicesShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> permuteShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> horizontalStepInverseFftShader = nullptr;
        CgEngine::ResRef<CgEngine::CustomComputePipeline> verticalStepInverseFftShader = nullptr;

        CgEngine::Attachment* buffer = nullptr;
        CgEngine::Attachment* twiddleFactors = nullptr;

        struct PCFft {
            int step;
            bool pingPong;
        };
        CgEngine::PushConstants* pushConstants = nullptr;

        std::unordered_map<CgEngine::Attachment*, std::unique_ptr<CgEngine::DescriptorSet>> fftDescriptorSets{};
        std::unordered_map<CgEngine::Attachment*, std::unique_ptr<CgEngine::DescriptorSet>> permuteDescriptorSets{};
    };
}
