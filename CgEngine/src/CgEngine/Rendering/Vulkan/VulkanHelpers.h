#pragma once

#include <vulkan/vulkan.hpp>
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    namespace VulkanHelpers {
        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t mipLevels, uint32_t layerCount, vk::ImageAspectFlags aspectFlags);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        size_t alignUp(size_t value, size_t alignment);
        vk::Format findSupportedDepthFormat(const std::vector<vk::Format>& candidates);
        bool hasFormatStencilComponent(vk::Format format);
        vk::Format attachmentTypeToVulkanColorFormat(AttachmentType attachmentType);
        vk::ImageAspectFlags attachmentTypeToAspectFlags(AttachmentType attachmentType);
        vk::ImageUsageFlags attachmentTypeToUsageFlags(bool usableAsTexture, AttachmentType attachmentType);
    }
}
