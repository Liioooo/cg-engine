#include "VulkanRenderPass.h"

namespace CgEngine {
    VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification &spec) : ready(true) {
        colorAttachmentLoadOp = spec.clearColorAttachments ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        depthStencilAttachmentLoadOp = spec.clearDepthStencilAttachment ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        colorAttachmentClearValue = {};
        colorAttachmentClearValue.color = {spec.clearColor.r, spec.clearColor.g, spec.clearColor.b, spec.clearColor.a};
    }

    VulkanRenderPass::VulkanRenderPass(VulkanRenderPass &&other) noexcept : RenderPass(std::move(other)) {
        ready = other.ready;
        colorAttachmentLoadOp = other.colorAttachmentLoadOp;
        depthStencilAttachmentLoadOp = other.depthStencilAttachmentLoadOp;
        colorAttachmentClearValue = other.colorAttachmentClearValue;
    }

    VulkanRenderPass& VulkanRenderPass::operator=(VulkanRenderPass &&other) noexcept {
        if (this != &other) {
            RenderPass::operator=(std::move(other));

            ready = other.ready;
            colorAttachmentLoadOp = other.colorAttachmentLoadOp;
            depthStencilAttachmentLoadOp = other.depthStencilAttachmentLoadOp;
            colorAttachmentClearValue = other.colorAttachmentClearValue;
        }
        return *this;
    }

    bool VulkanRenderPass::isReady() const {
        return ready;;
    }

    vk::AttachmentLoadOp VulkanRenderPass::getColorAttachmentLoadOp() const {
        return colorAttachmentLoadOp;
    }

    vk::AttachmentLoadOp VulkanRenderPass::getDepthStencilAttachmentLoadOp() const {
        return depthStencilAttachmentLoadOp;
    }

    vk::ClearValue VulkanRenderPass::getColorAttachmentClearValue() const {
        return colorAttachmentClearValue;
    }
}
