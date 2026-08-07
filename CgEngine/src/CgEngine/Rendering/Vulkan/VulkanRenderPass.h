#pragma once

#include "Rendering/RenderPass.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {
    class VulkanRenderPass : public RenderPass {
    public:
        VulkanRenderPass() = default;
        explicit VulkanRenderPass(const RenderPassSpecification& spec);

        ~VulkanRenderPass() override = default;

        VulkanRenderPass(VulkanRenderPass&& other) noexcept;
        VulkanRenderPass& operator= (VulkanRenderPass&& other) noexcept;

        VulkanRenderPass(VulkanRenderPass& other) = delete;
        VulkanRenderPass& operator=(VulkanRenderPass& other) = delete;

        bool isReady() const override;

        vk::AttachmentLoadOp getColorAttachmentLoadOp() const;
        vk::AttachmentLoadOp getDepthStencilAttachmentLoadOp() const;
        vk::ClearValue getColorAttachmentClearValue() const;

    private:
        bool ready = false;
        vk::AttachmentLoadOp colorAttachmentLoadOp;
        vk::AttachmentLoadOp depthStencilAttachmentLoadOp;
        vk::ClearValue colorAttachmentClearValue;
    };
}
