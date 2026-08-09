#pragma once

#include "vk_mem_alloc.h"
#include "Rendering/Attachment.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {

    struct VulkanAttachmentState {
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
        vk::AccessFlags2 access = {};
        vk::PipelineStageFlags2 stage = {};
    };

    class VulkanAttachment : public Attachment {
    public:
        VulkanAttachment() = default;
        explicit VulkanAttachment(const AttachmentSpecification& spec);

        ~VulkanAttachment() override;

        VulkanAttachment(VulkanAttachment&& other) noexcept;
        VulkanAttachment& operator=(VulkanAttachment&& other) noexcept;

        VulkanAttachment(VulkanAttachment& other) = delete;
        VulkanAttachment& operator=(VulkanAttachment& other) = delete;

        AttachmentType getType() const override;
        DepthStencilAttachmentFormat getDepthStencilAttachmentFormat() const override;
        bool isUsableAsTexture() const override;
        bool isUsableAsStorageImage() const override;
        uint32_t getLayerCount() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

        void resize(uint32_t newWidth, uint32_t newHeight) override;

        vk::Image getVulkanImage() const;
        vk::ImageView getVulkanImageView() const;
        vk::ImageView getVulkanLayerImageView(uint32_t layer) const;
        vk::Sampler getVulkanSampler() const;
        vk::ImageAspectFlags getVulkanAspectFlags() const;
        std::vector<VulkanAttachmentState>& getSubresourceStates();

        void deferredDestroyCurrentResources();

    private:
        vk::Image image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        vk::ImageView imageView = VK_NULL_HANDLE;
        std::vector<vk::ImageView> layerImageViews;
        DepthStencilAttachmentFormat depthFormat;
        AttachmentType type;
        bool usableAsTexture = false;
        bool usableAsStorageImage = false;
        uint32_t layerCount;
        uint32_t width = 0;
        uint32_t height = 0;
        vk::Format vulkanFormat;
        std::vector<VulkanAttachmentState> subresourceStates{};

        // Owned by VulkanSamplerManager
        vk::Sampler sampler = VK_NULL_HANDLE;

        void createAttachmentImage();
        void createImageViews();
    };
}