#pragma once
#include "Rendering/TextureCube.h"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace CgEngine {
    class VulkanTextureCube : public TextureCube {
    public:
        VulkanTextureCube() = default;
        VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);
        VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);

        ~VulkanTextureCube() override;

        VulkanTextureCube(VulkanTextureCube&& other) noexcept;
        VulkanTextureCube& operator=(VulkanTextureCube&& other) noexcept;

        VulkanTextureCube(VulkanTextureCube& other) = delete;
        VulkanTextureCube& operator=(VulkanTextureCube& other) = delete;

        bool isReady() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;
        TextureFormat getFormat() const override;

        void generateMipMaps() override;

        vk::ImageView getVulkanImageView() const;
        vk::Sampler getVulkanSampler() const;
        vk::ImageView createStorageImageView(uint32_t mipLevel) const;
        void recordLayoutTransition(const vk::CommandBuffer& commandBuffer, uint32_t baseMipLevel, uint32_t levelCount, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess) const;

        void deferredDestroyCurrentResources();

    private:
        static constexpr uint32_t FACE_COUNT = 6;

        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format{};
        uint32_t mipLevels = 1;

        vk::Image image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        vk::ImageView imageView = VK_NULL_HANDLE;

        // Owned by VulkanSamplerManager
        vk::Sampler sampler = VK_NULL_HANDLE;

        void createVulkanTextureCube(const void* data, MipMapFiltering mipMapFiltering);
        void recordMipMapGeneration(const vk::CommandBuffer& commandBuffer) const;
    };
}