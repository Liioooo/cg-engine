#include "VulkanAttachment.h"
#include "Asserts.h"
#include "VulkanHelpers.h"
#include "Rendering/Renderer.h"
#include "VulkanRenderer.h"


namespace CgEngine {
    VulkanAttachment::VulkanAttachment(const AttachmentSpecification &spec) : usableAsTexture(spec.usableAsTexture), usableAsStorageImage(spec.usableAsStorageImage), layerCount(spec.layerCount), width(spec.width), height(spec.height) {
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

        subresourceStates.resize(layerCount);

        if (usableAsTexture) {
            CG_ASSERT(spec.mipMapFiltering != MipMapFiltering::Trilinear, "Trilinear filtering is not supported for Attachments!")
            CG_ASSERT(spec.mipMapFiltering != MipMapFiltering::Anisotropic, "Anisotropic filtering is not supported for Attachments!")
            sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(spec.textureWrap, spec.mipMapFiltering, spec.textureBorderColor);
        }
    }

    VulkanAttachment::~VulkanAttachment() {
        deferredDestroyCurrentResources();
    }

    VulkanAttachment::VulkanAttachment(VulkanAttachment &&other) noexcept : Attachment(std::move(other)) {
        image = other.image;
        allocation = other.allocation;
        imageView = other.imageView;
        layerImageViews = std::move(other.layerImageViews);
        depthFormat = other.depthFormat;
        type = other.type;
        usableAsTexture = other.usableAsTexture;
        usableAsStorageImage = other.usableAsStorageImage;
        layerCount = other.layerCount;
        width = other.width;
        height = other.height;
        vulkanFormat = other.vulkanFormat;
        subresourceStates = other.subresourceStates;
        sampler = other.sampler;

        other.image = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.sampler = VK_NULL_HANDLE;
    }

    VulkanAttachment & VulkanAttachment::operator=(VulkanAttachment &&other) noexcept {
        if (this != &other) {
            Attachment::operator=(std::move(other));

            deferredDestroyCurrentResources();

            image = other.image;
            allocation = other.allocation;
            imageView = other.imageView;
            layerImageViews = std::move(other.layerImageViews);
            depthFormat = other.depthFormat;
            type = other.type;
            usableAsTexture = other.usableAsTexture;
            usableAsStorageImage = other.usableAsStorageImage;
            layerCount = other.layerCount;
            width = other.width;
            height = other.height;
            vulkanFormat = other.vulkanFormat;
            subresourceStates = other.subresourceStates;
            sampler = other.sampler;

            other.image = VK_NULL_HANDLE;
            other.imageView = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
            other.sampler = VK_NULL_HANDLE;
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

    bool VulkanAttachment::isUsableAsStorageImage() const {
        return usableAsStorageImage;
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
        deferredDestroyCurrentResources();

        width = newWidth;
        height = newHeight;

        createAttachmentImage();
        createImageViews();

        subresourceStates.clear();
        subresourceStates.resize(layerCount);
    }

    vk::Image VulkanAttachment::getVulkanImage() const {
        return image;
    }

    vk::ImageView VulkanAttachment::getVulkanImageView() const {
        CG_ASSERT(imageView != VK_NULL_HANDLE, "Attachment::getVulkanImageView: Attachment has not been created properly.")
        return imageView;
    }
    vk::ImageView VulkanAttachment::getVulkanLayerImageView(uint32_t layer) const {
        CG_ASSERT(layer < layerCount, "Attachment::getVulkanLayerImageView: Layer index out of bounds.")

        if (layer == 0 && layerCount == 1) {
            CG_ASSERT(imageView != VK_NULL_HANDLE, "Attachment::getVulkanLayerImageView: Attachment has not been created properly.")
            return imageView;
        }

        CG_ASSERT(!layerImageViews.empty(), "Attachment::getVulkanLayerImageView: Attachment has not been created properly.")
        CG_ASSERT(layerImageViews[layer] != VK_NULL_HANDLE, "Attachment::getVulkanLayerImageView: Attachment has not been created properly.")
        return layerImageViews[layer];
    }

    vk::Sampler VulkanAttachment::getVulkanSampler() const {
        CG_ASSERT(sampler != VK_NULL_HANDLE, "Attachment::getVulkanSampler: Attachment has not been created properly.")
        return sampler;
    }

    vk::ImageAspectFlags VulkanAttachment::getVulkanAspectFlags() const {
        return VulkanHelpers::attachmentTypeToAspectFlags(type);
    }


    std::vector<VulkanAttachmentState>& VulkanAttachment::getSubresourceStates() {
        return subresourceStates;
    }

    void VulkanAttachment::createAttachmentImage() {
        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        vk::ImageUsageFlags usageFlags = VulkanHelpers::attachmentTypeToUsageFlags(usableAsTexture, usableAsStorageImage, type);

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
        imageCreateInfo.flags = {};

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
        vmaSetAllocationName(allocator, allocation, "VulkanAttachment");

        image = rawImage;
    }

    void VulkanAttachment::createImageViews() {
        vk::ImageAspectFlags aspectFlags = VulkanHelpers::attachmentTypeToAspectFlags(type);

        imageView = VulkanHelpers::createImageView2D(image, vulkanFormat, 1, layerCount, aspectFlags);

        if (layerCount > 1) {
            layerImageViews.resize(layerCount);
            for (uint32_t layer = 0; layer < layerCount; ++layer) {
                layerImageViews[layer] = VulkanHelpers::createImageView2D(image, vulkanFormat, 1, 1, aspectFlags, layer);
            }
        }
    }

    void VulkanAttachment::deferredDestroyCurrentResources() {
        if (!image && !imageView && layerImageViews.empty()) {
            return;
        }

        vk::Image oldImage = image;
        VmaAllocation oldAllocation = allocation;
        vk::ImageView oldImageView = imageView;
        std::vector<vk::ImageView> oldLayerImageViews = std::move(layerImageViews);

        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        imageView = VK_NULL_HANDLE;
        layerImageViews.clear();

        Renderer::getVulkanBackend()->deferDestruction([oldImage, oldAllocation, oldImageView, oldLayerImageViews] {
            auto device = Renderer::getVulkanBackend()->getVkDevice();

            for (auto view : oldLayerImageViews) {
                device.destroyImageView(view);
            }
            if (oldImageView) {
                device.destroyImageView(oldImageView);
            }
            if (oldImage) {
                VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
                vmaDestroyImage(allocator, oldImage, oldAllocation);
            }
        });
    }
}
