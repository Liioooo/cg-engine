#include "VulkanAttachment.h"

#include "VulkanHelpers.h"
#include "Rendering/Renderer.h"
#include "VulkanRenderer.h"


namespace CgEngine {
    VulkanAttachment::VulkanAttachment(const AttachmentSpecification &spec) : usableAsTexture(spec.usableAsTexture), mipMapFiltering(spec.mipMapFiltering), textureWrap(spec.textureWrap), textureBorderColor(spec.textureBorderColor), layerCount(spec.layerCount), width(spec.width), height(spec.height) {
        if (spec.type == AttachmentType::Depth || spec.type == AttachmentType::DepthStencil) {
            if (spec.type == AttachmentType::Depth) {
                vulkanFormat = VulkanHelpers::findSupportedDepthFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});
            } else {
                vulkanFormat = VulkanHelpers::findSupportedDepthFormat({vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint});
            }
            type = VulkanHelpers::hasFormatStencilComponent(vulkanFormat) ? AttachmentType::DepthStencil : AttachmentType::Depth;

            switch (vulkanFormat) {
                case vk::Format::eD24UnormS8Uint:
                    depthFormat = DepthStencilAttachmentFormat::Depth24Stencil8;
                    break;
                case vk::Format::eD32SfloatS8Uint:
                    depthFormat = DepthStencilAttachmentFormat::Depth32FloatStencil8;
                    break;
                case vk::Format::eD32Sfloat:
                    depthFormat = DepthStencilAttachmentFormat::Depth32Float;
                    break;
            }

        } else {
            type = spec.type;
            vulkanFormat = VulkanHelpers::attachmentTypeToVulkanColorFormat(spec.type);
        }

        createAttachmentImage();
        createImageViews();
    }

    VulkanAttachment::~VulkanAttachment() {
        auto device = Renderer::getVulkanBackend()->getVkDevice();

        for (auto view : layerImageViews) {
            device.destroyImageView(view);
        }
        if (imageView) {
            device.destroyImageView(imageView);
        }
        if (image) {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyImage(allocator, image, allocation);
        }

    }

    VulkanAttachment::VulkanAttachment(VulkanAttachment &&other) noexcept : Attachment(std::move(other)) {
        image = other.image;
        allocation = other.allocation;
        imageView = other.imageView;
        layerImageViews = std::move(other.layerImageViews);
        depthFormat = other.depthFormat;
        type = other.type;
        usableAsTexture = other.usableAsTexture;
        textureWrap = other.textureWrap;
        mipMapFiltering = other.mipMapFiltering;
        textureBorderColor = other.textureBorderColor;
        layerCount = other.layerCount;
        width = other.width;
        height = other.height;
        vulkanFormat = other.vulkanFormat;

        other.image = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanAttachment & VulkanAttachment::operator=(VulkanAttachment &&other) noexcept {
        if (this != &other) {
            Attachment::operator=(std::move(other));

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            for (auto view : layerImageViews) {
                device.destroyImageView(view);
            }
            if (imageView) {
                device.destroyImageView(imageView);
            }
            if (image) {
                VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
                vmaDestroyImage(allocator, image, allocation);
            }

            image = other.image;
            allocation = other.allocation;
            imageView = other.imageView;
            layerImageViews = std::move(other.layerImageViews);
            depthFormat = other.depthFormat;
            type = other.type;
            usableAsTexture = other.usableAsTexture;
            textureWrap = other.textureWrap;
            mipMapFiltering = other.mipMapFiltering;
            textureBorderColor = other.textureBorderColor;
            layerCount = other.layerCount;
            width = other.width;
            height = other.height;
            vulkanFormat = other.vulkanFormat;

            other.image = VK_NULL_HANDLE;
            other.imageView = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }
        return *this;
    }

    AttachmentType VulkanAttachment::getType() const {
        return type;
    }

    DepthStencilAttachmentFormat VulkanAttachment::getDepthStencilAttachmentFormat() const {
        return depthFormat;
    }

    bool VulkanAttachment::isUsableAsTexture() const {
        return usableAsTexture;
    }

    uint32_t VulkanAttachment::getLayerCount() const {
        return layerCount;
    }

    uint32_t VulkanAttachment::getWidth() const {
        return width;
    }

    uint32_t VulkanAttachment::getHeight() const {
        return height;
    }

    void VulkanAttachment::resize(uint32_t newWidth, uint32_t newHeight) {
         auto device = Renderer::getVulkanBackend()->getVkDevice();

        for (auto view : layerImageViews) {
            device.destroyImageView(view);
        }
        if (imageView) {
            device.destroyImageView(imageView);
        }
        if (image) {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyImage(allocator, image, allocation);
        }

        width = newWidth;
        height = newHeight;

        createAttachmentImage();
        createImageViews();
    }

    vk::ImageView VulkanAttachment::getVulkanImageView() const {
        CG_ASSERT(imageView != VK_NULL_HANDLE, "Attachment::getVulkanImageView: Attachment has not been created properly.")
        return imageView;
    }

    vk::ImageView VulkanAttachment::getVulkanLayerImageView(uint32_t layer) const {
        CG_ASSERT(layer < layerCount, "Attachment::getVulkanLayerImageView: Layer index out of bounds.")
        CG_ASSERT(!layerImageViews.empty(), "Attachment::getVulkanLayerImageView: Attachment has not been created properly.")
        CG_ASSERT(layerImageViews[layer] != VK_NULL_HANDLE, "Attachment::getVulkanLayerImageView: Attachment has not been created properly.")
        return layerImageViews[layer];
    }

    void VulkanAttachment::createAttachmentImage() {
        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        vk::ImageUsageFlags usageFlags = VulkanHelpers::attachmentTypeToUsageFlags(usableAsTexture, type);

        vk::ImageCreateFlags createFlags = {};
        if (layerCount > 1) {
            createFlags |= vk::ImageCreateFlagBits::e2DArrayCompatible;
        }

        vk::ImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.format = vulkanFormat;
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = layerCount;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.usage = usageFlags;
        imageCreateInfo.flags = createFlags;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkImage rawImage{};

        vmaCreateImage(
            allocator,
            reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo),
            &allocInfo,
            &rawImage,
            &allocation,
            nullptr
        );

        image = rawImage;
    }

    void VulkanAttachment::createImageViews() {
        auto device = Renderer::getVulkanBackend()->getVkDevice();

        vk::ImageAspectFlags aspectFlags = VulkanHelpers::attachmentTypeToAspectFlags(type);
        vk::ImageViewType viewType = (layerCount > 1) ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;

        vk::ImageSubresourceRange range{};
        range.aspectMask = aspectFlags;
        range.layerCount = layerCount;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;

        vk::ImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.flags = {};
        viewCreateInfo.image = image;
        viewCreateInfo.viewType = viewType;
        viewCreateInfo.format = vulkanFormat;
        viewCreateInfo.subresourceRange = range;
        viewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

        auto imageViewResult = device.createImageView(viewCreateInfo);
        if (imageViewResult.has_value()) {
            imageView = imageViewResult.value;
        } else {
            CG_LOGGING_ERROR("Failed to create Vulkan image view for attachment!")
        }

        layerImageViews.resize(layerCount);
        for (uint32_t layer = 0; layer < layerCount; ++layer) {
            vk::ImageViewCreateInfo layerViewCreateInfo{};
            layerViewCreateInfo.flags = {};
            layerViewCreateInfo.image = image;
            layerViewCreateInfo.viewType = vk::ImageViewType::e2D;
            layerViewCreateInfo.format = vulkanFormat;
            layerViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
            layerViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
            layerViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
            layerViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
            layerViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
            layerViewCreateInfo.subresourceRange.layerCount = 1;
            layerViewCreateInfo.subresourceRange.baseMipLevel = 0;
            layerViewCreateInfo.subresourceRange.levelCount = 1;
            layerViewCreateInfo.subresourceRange.baseArrayLayer = layer;

            auto layerImageViewResult = device.createImageView(layerViewCreateInfo);
            if (layerImageViewResult.has_value()) {
                layerImageViews[layer] = layerImageViewResult.value;
            } else {
                CG_LOGGING_ERROR("Failed to create Vulkan image view for attachment!")
            }
        }
    }
}
