#pragma once
#include "Rendering/ComputePipeline.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {
    class VulkanComputePipeline : public ComputePipeline {
    public:
        VulkanComputePipeline() = default;
        explicit VulkanComputePipeline(const ComputePipelineSpecification& spec);

        ~VulkanComputePipeline() override;

        VulkanComputePipeline(VulkanComputePipeline&& other) noexcept;
        VulkanComputePipeline& operator=(VulkanComputePipeline&& other) noexcept;

        VulkanComputePipeline(VulkanComputePipeline& other) = delete;
        VulkanComputePipeline& operator=(VulkanComputePipeline& other) = delete;

        bool isReady() const override;

        vk::Pipeline getVulkanPipeline() const;
        vk::PipelineLayout getVulkanPipelineLayout() const;

    private:
        vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;
    };
}
