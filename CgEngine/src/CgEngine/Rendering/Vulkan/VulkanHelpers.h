#pragma once

#include <vulkan/vulkan.hpp>
#include "CgEngineSharedUtils/Enums.h"
#include "Rendering/VertexBuffer.h"

namespace CgEngine {

    namespace VulkanHelpers {
        struct VulkanGraphicsShaderInfo {
            vk::ShaderModule vertexModule = VK_NULL_HANDLE;
            vk::ShaderModule fragmentModule = VK_NULL_HANDLE;
            vk::ShaderModule geometryModule = VK_NULL_HANDLE;
            vk::ShaderModule tesModule = VK_NULL_HANDLE;
            vk::ShaderModule tcsModule = VK_NULL_HANDLE;
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        };

        struct VulkanPipelineVertexInputInfo {
            vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
            std::vector<vk::VertexInputBindingDescription> bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
        };

        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t mipLevels, uint32_t layerCount, vk::ImageAspectFlags aspectFlags);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        size_t alignUp(size_t value, size_t alignment);
        vk::Format findSupportedDepthFormat(const std::vector<vk::Format>& candidates);
        bool hasFormatStencilComponent(vk::Format format);
        vk::Format attachmentTypeToVulkanColorFormat(AttachmentType attachmentType);
        vk::ImageAspectFlags attachmentTypeToAspectFlags(AttachmentType attachmentType);
        vk::ImageUsageFlags attachmentTypeToUsageFlags(bool usableAsTexture, AttachmentType attachmentType);
        VulkanGraphicsShaderInfo loadVulkanGraphicsShader(const std::string& name, ShaderEnv env);
        VulkanGraphicsShaderInfo loadVulkanGraphicsShader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& tcs, const std::string& tes, ShaderEnv env);
        VulkanGraphicsShaderInfo createVulkanGraphicsShaderInfoFromSources(const std::vector<uint8_t>& vertexSource, const std::vector<uint8_t>& fragmentSource, const std::vector<uint8_t>& geometrySource, const std::vector<uint8_t>& tcsSource, const std::vector<uint8_t>& tesSource);
        vk::ShaderModule createVulkanShaderModule(const std::vector<uint8_t>& binary);
        void destroyVulkanGraphicsShaderModules(VulkanGraphicsShaderInfo& shaderInfo);
        VulkanPipelineVertexInputInfo getVulkanPipelineVertexInputInfo(const std::vector<VertexBufferLayout>& vertexInputLayout);
        vk::Format shaderDataTypeToVulkanFormat(ShaderDataType type);
        vk::PrimitiveTopology drawModeToVulkanPrimitiveTopology(DrawMode mode);
        vk::CompareOp depthCompareOperatorToVulkan(DepthCompareOperator op);
        vk::BlendOp blendingEquationToVulkan(BlendingEquation eq);
        vk::BlendFactor blendingFunctionToVulkan(BlendingFunction fn);
        vk::Format depthStencilAttachmentFormatToVulkanFormat(DepthStencilAttachmentFormat format);
    }
}
