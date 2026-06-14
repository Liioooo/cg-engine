#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
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

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
            auto backend = Renderer::getVulkanBackend();

            vk::CommandBuffer commandBuffer = backend->beginSingleTimeCommandBuffer();

            vk::BufferCopy2 copyRegion{};
            copyRegion.setSize(size);
            copyRegion.setSrcOffset(0);
            copyRegion.setDstOffset(0);

            vk::CopyBufferInfo2 copyRegionInfo{};
            copyRegionInfo.setSrcBuffer(srcBuffer);
            copyRegionInfo.setDstBuffer(dstBuffer);
            copyRegionInfo.setPRegions(&copyRegion);
            copyRegionInfo.setRegionCount(1);

            commandBuffer.copyBuffer2(&copyRegionInfo);

            backend->endAndSubmitSingleTimeCommandBuffer(commandBuffer);
        }

        size_t alignUp(size_t value, size_t alignment) {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        vk::Format findSupportedDepthFormat(const std::vector<vk::Format>& candidates) {
            auto physicalDevice = Renderer::getVulkanBackend()->getVkPhysicalDevice();

            for (vk::Format format : candidates) {
                vk::FormatProperties props = physicalDevice.getFormatProperties(format);

                if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                    return format;
                }
            }
            CG_LOGGING_ERROR("failed to find supported depth format!")
            return vk::Format::eUndefined;
        }

        bool hasFormatStencilComponent(vk::Format format) {
            return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD16UnormS8Uint;
        }

        vk::Format attachmentTypeToVulkanColorFormat(AttachmentType attachmentType) {
            switch (attachmentType) {
                case AttachmentType::RGBA8:
                    return vk::Format::eR8G8B8A8Unorm;
                case AttachmentType::RGBA16F:
                    return vk::Format::eR16G16B16A16Sfloat;
                case AttachmentType::RGBA32F:
                    return vk::Format::eR32G32B32A32Sfloat;
                case AttachmentType::RG8:
                    return vk::Format::eR8G8Unorm;
                case AttachmentType::RG16F:
                    return vk::Format::eR16G16Sfloat;
                case AttachmentType::RG32F:
                    return vk::Format::eR32G32Sfloat;
                case AttachmentType::R16F:
                    return vk::Format::eR16Sfloat;
            }

            CG_LOGGING_ERROR("Given attachment type is not a color attachment!")
            return vk::Format::eR8G8B8A8Unorm;;
        }

        vk::ImageAspectFlags attachmentTypeToAspectFlags(AttachmentType attachmentType) {
            switch (attachmentType) {
                case AttachmentType::Depth:
                    return vk::ImageAspectFlagBits::eDepth;
                case AttachmentType::DepthStencil:
                    return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
                default:
                    return vk::ImageAspectFlagBits::eColor;
            }
        }

        vk::ImageUsageFlags attachmentTypeToUsageFlags(bool usableAsTexture, AttachmentType attachmentType) {
            vk::ImageUsageFlags flags = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eColorAttachment;

            if (usableAsTexture) {
                flags |= vk::ImageUsageFlagBits::eSampled;
            }

            if (attachmentType == AttachmentType::Depth || attachmentType == AttachmentType::DepthStencil) {
                flags &= ~vk::ImageUsageFlagBits::eColorAttachment;
                flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            }

            return flags;
        }
    }
}
