#include "VulkanHelpers.h"
#include "Rendering/Renderer.h"

namespace CgEngine {

    namespace VulkanHelpers {
        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t mipLevels, uint32_t layerCount, vk::ImageAspectFlags aspectFlags) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = image;
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = format;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask = aspectFlags;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = mipLevels;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = layerCount;

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            auto result = device.createImageView(createInfo);
            CG_ASSERT(result.has_value(), "VulkanHelpers::createImageView2D: Failed to create image view.")
            return result.value;
        }
    }
}
