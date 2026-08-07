#pragma once
#include "Rendering/GraphicsPipeline.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {
    class VulkanGraphicsPipeline : public GraphicsPipeline {
    public:
        VulkanGraphicsPipeline() = default;
        explicit VulkanGraphicsPipeline(const GraphicsPipelineSpecification& spec);

        ~VulkanGraphicsPipeline() override;

        VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept;
        VulkanGraphicsPipeline& operator= (VulkanGraphicsPipeline&& other) noexcept;

        VulkanGraphicsPipeline(VulkanGraphicsPipeline& other) = delete;
        VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline& other) = delete;

        bool isReady() const override;

        vk::Pipeline getVulkanPipeline() const;
        vk::PipelineLayout getVulkanPipelineLayout() const;
        vk::ShaderStageFlags getShaderStageFlags() const;

    private:
        vk::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;
        vk::ShaderStageFlags shaderStageFlags{};
    };
}
