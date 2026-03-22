#pragma once

#include <vulkan/vulkan.hpp>

namespace CgEngine {

    namespace VulkanHelpers {
        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t mipLevels, uint32_t layerCount, vk::ImageAspectFlags aspectFlags);
        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    }
}
