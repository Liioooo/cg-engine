#pragma once
#include "Rendering/Framebuffer.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {
    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer() = default;
        VulkanFramebuffer(const FramebufferSpecification& spec);

        ~VulkanFramebuffer() override;

        VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
        VulkanFramebuffer& operator=(VulkanFramebuffer&& other) noexcept;

        VulkanFramebuffer(VulkanFramebuffer& other) = delete;
        VulkanFramebuffer& operator=(VulkanFramebuffer& other) = delete;

        void recreate(uint32_t newWidth, uint32_t newHeight) override;

        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

        const std::vector<FramebufferAttachment>& getColorFramebufferAttachments() const override;
        const FramebufferAttachment& getDepthStencilFramebufferAttachment() const override;

        bool isReady() const override;

        const std::vector<vk::ImageView>& getVulkanColorAttachmentImageViews() const;
        vk::ImageView getVulkanDepthAttachmentImageView() const;
        vk::ImageView getVulkanStencilAttachmentImageView() const;
        uint32_t getLayerCount() const;

    private:
        uint32_t width = 0;
        uint32_t height = 0;
        bool ready = false;

        uint32_t layers = 1;
        std::vector<vk::ImageView> colorAttachmentImageViews;
        vk::ImageView depthAttachmentImageView = VK_NULL_HANDLE;
        vk::ImageView stencilAttachmentImageView = VK_NULL_HANDLE;

        std::vector<FramebufferAttachment> colorAttachments;
        FramebufferAttachment depthStencilAttachment;

        void init();
    };
}
