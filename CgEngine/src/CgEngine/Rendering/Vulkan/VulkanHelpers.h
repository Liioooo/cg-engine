#pragma once

#include <vulkan/vulkan.hpp>
#include "Rendering/Vulkan/VulkanRenderer.h"

namespace CgEngine {

    namespace VulkanHelpers {
        VulkanRenderer* getVulkanRenderer();
        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t mipLevels, uint32_t layerCount, vk::ImageAspectFlags aspectFlags);
    }
}
