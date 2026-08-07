#pragma once

#include "DescriptorSetLayout.h"


namespace CgEngine {

    struct ComputePipelineSpecification {
        std::vector<const DescriptorSetLayout*> descriptorSetLayouts;
        std::string engineShaderName;
        std::string customShader;
        bool usesPushConstants = false;
        uint32_t pushConstantsSize = 0;
    };

    class ComputePipeline {
    public:
        ComputePipeline() = default;

        virtual ~ComputePipeline() = default;

        ComputePipeline(ComputePipeline&& other) noexcept = default;
        ComputePipeline& operator= (ComputePipeline&& other) noexcept = default;

        ComputePipeline(ComputePipeline& other) = delete;
        ComputePipeline& operator=(ComputePipeline& other) = delete;

        virtual bool isReady() const = 0;
    };

}
