#pragma once

#include <vulkan/vulkan.hpp>
#include "CgEngineSharedUtils/Enums.h"
#include "Rendering/DescriptorSetLayout.h"
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

        struct VulkanComputeShaderInfo {
            vk::ShaderModule computeModule = VK_NULL_HANDLE;
            vk::PipelineShaderStageCreateInfo shaderStage;
        };

        struct VulkanPipelineVertexInputInfo {
            vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
            std::vector<vk::VertexInputBindingDescription> bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
        };

        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t levelCount, uint32_t layerCount, vk::ImageAspectFlags aspectFlags, uint32_t baseArrayLayer = 0);
        vk::ImageView createImageViewCube(vk::Image image, vk::Format format, uint32_t levelCount, vk::ImageAspectFlags aspectFlags, uint32_t baseMipLevel = 0);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        size_t alignUp(size_t value, size_t alignment);
        vk::Format findSupportedDepthFormat(const std::vector<vk::Format>& candidates);
        bool hasFormatStencilComponent(vk::Format format);
        vk::Format attachmentTypeToVulkanColorFormat(AttachmentType attachmentType);
        AttachmentType vkColorFormatToAttachmentType(vk::Format format);
        vk::ImageAspectFlags attachmentTypeToAspectFlags(AttachmentType attachmentType);
        vk::ImageUsageFlags attachmentTypeToUsageFlags(bool usableAsTexture, bool usableAsStorageImage, AttachmentType attachmentType);
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
        vk::ShaderStageFlags descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(DescriptorSetLayoutBindingUsage usage);
        vk::PipelineStageFlags2 descriptorSetLayoutBindingUsageToVulkanPipelineStageFlags(DescriptorSetLayoutBindingUsage usage);
        vk::Format textureFormatToVulkanFormat(TextureFormat format);
        VulkanComputeShaderInfo loadVulkanComputeShader(const std::string& name, ShaderEnv env);
        VulkanComputeShaderInfo loadVulkanCustomComputeShader(const std::string& name);
        VulkanComputeShaderInfo createVulkanComputeShaderInfoFromSource(const std::vector<uint8_t>& source);
        void destroyVulkanComputeShaderModule(VulkanComputeShaderInfo& shaderInfo);
    }
}
