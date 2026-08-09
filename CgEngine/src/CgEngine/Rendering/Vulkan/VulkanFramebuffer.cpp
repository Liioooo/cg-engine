#include "VulkanFramebuffer.h"

#include "Asserts.h"
#include "VulkanAttachment.h"

namespace CgEngine {
    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification &spec) : width(spec.width), height(spec.height), ready(true), colorAttachments(spec.colorAttachments), depthStencilAttachment(spec.depthStencilAttachment) {
        init();
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        ready = false;
    }

    VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer &&other) noexcept : Framebuffer(std::move(other)) {
        height = other.height;
        width = other.width;
        ready = other.ready;
        layers = other.layers;
        colorAttachmentImageViews = std::move(other.colorAttachmentImageViews);
        depthAttachmentImageView = other.depthAttachmentImageView;
        stencilAttachmentImageView = other.stencilAttachmentImageView;
        colorAttachments = std::move(other.colorAttachments);
        depthStencilAttachment = other.depthStencilAttachment;

        other.ready = false;
        other.depthAttachmentImageView = VK_NULL_HANDLE;
        other.stencilAttachmentImageView = VK_NULL_HANDLE;
        other.depthStencilAttachment = {};
    }

    VulkanFramebuffer & VulkanFramebuffer::operator=(VulkanFramebuffer &&other) noexcept {
        if (this != &other) {
            Framebuffer::operator=(std::move(other));

            height = other.height;
            width = other.width;
            ready = other.ready;
            layers = other.layers;
            colorAttachmentImageViews = std::move(other.colorAttachmentImageViews);
            depthAttachmentImageView = other.depthAttachmentImageView;
            stencilAttachmentImageView = other.stencilAttachmentImageView;
            colorAttachments = std::move(other.colorAttachments);
            depthStencilAttachment = other.depthStencilAttachment;

            other.ready = false;
            other.depthAttachmentImageView = VK_NULL_HANDLE;
            other.stencilAttachmentImageView = VK_NULL_HANDLE;
            other.depthStencilAttachment = {};
        }
        return *this;
    }

    void VulkanFramebuffer::recreate(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        init();
    }

    uint32_t VulkanFramebuffer::getWidth() const {
        return width;
    }

    uint32_t VulkanFramebuffer::getHeight() const {
        return height;
    }

    const std::vector<FramebufferAttachment> & VulkanFramebuffer::getColorFramebufferAttachments() const {
        return colorAttachments;
    }

    const FramebufferAttachment& VulkanFramebuffer::getDepthStencilFramebufferAttachment() const {
        return depthStencilAttachment;
    }

    bool VulkanFramebuffer::isReady() const {
        return ready;
    }

    const std::vector<vk::ImageView>& VulkanFramebuffer::getVulkanColorAttachmentImageViews() const {
        return colorAttachmentImageViews;
    }

    vk::ImageView VulkanFramebuffer::getVulkanDepthAttachmentImageView() const {
        return depthAttachmentImageView;
    }

    vk::ImageView VulkanFramebuffer::getVulkanStencilAttachmentImageView() const {
        return stencilAttachmentImageView;
    }

    uint32_t VulkanFramebuffer::getLayerCount() const {
        return layers;
    }

    void VulkanFramebuffer::init() {
        CG_ASSERT(width > 0 && height > 0, "Framebuffer width and height must be greater than 0!")
        CG_ASSERT(!colorAttachments.empty() || depthStencilAttachment.attachment != nullptr, "At least one attachment (color or depth) must be provided!")

        colorAttachmentImageViews.resize(colorAttachments.size());
        for (size_t i = 0; i < colorAttachments.size(); i++) {
            const auto* attachment = static_cast<const VulkanAttachment*>(colorAttachments[i].attachment);
            if (colorAttachments[i].allLayers) {
                colorAttachmentImageViews[i] = attachment->getVulkanImageView();
                layers = attachment->getLayerCount();
            } else {
                colorAttachmentImageViews[i] = attachment->getVulkanLayerImageView(colorAttachments[i].layer);
            }
        }

        if (depthStencilAttachment.attachment) {
            const auto* attachment = static_cast<const VulkanAttachment*>(depthStencilAttachment.attachment);
            CG_ASSERT(attachment->getType() == AttachmentType::Depth || attachment->getType() == AttachmentType::DepthStencil, "DepthStencil attachment must be of type Depth or DepthStencil")

            if (attachment->getType() == AttachmentType::DepthStencil) {
                if (depthStencilAttachment.allLayers) {
                    depthAttachmentImageView = attachment->getVulkanImageView();
                    stencilAttachmentImageView = attachment->getVulkanImageView();
                    layers = attachment->getLayerCount();
                } else {
                    depthAttachmentImageView = attachment->getVulkanLayerImageView(depthStencilAttachment.layer);
                    stencilAttachmentImageView = attachment->getVulkanLayerImageView(depthStencilAttachment.layer);
                }
            } else {
                if (depthStencilAttachment.allLayers) {
                    depthAttachmentImageView = attachment->getVulkanImageView();
                    layers = attachment->getLayerCount();
                } else {
                    depthAttachmentImageView = attachment->getVulkanLayerImageView(depthStencilAttachment.layer);
                }
                stencilAttachmentImageView = VK_NULL_HANDLE;
            }
        }
    }
}
