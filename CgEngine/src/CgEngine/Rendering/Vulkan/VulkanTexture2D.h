#pragma once

#include "Rendering/Texture2D.h"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"


namespace CgEngine {
    class VulkanTexture2D : public Texture2D {
    public:
        VulkanTexture2D() = default;
        VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);
        VulkanTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);
        VulkanTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering= MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);

        ~VulkanTexture2D() override;

        VulkanTexture2D(VulkanTexture2D&& other) noexcept;
        VulkanTexture2D& operator=(VulkanTexture2D&& other) noexcept;

        VulkanTexture2D(VulkanTexture2D& other) = delete;
        VulkanTexture2D& operator=(VulkanTexture2D& other) = delete;

        uint32_t getWidth() const override;
        uint32_t getHeight() const override;
        TextureFormat getFormat() const override;

        vk::ImageView getVulkanImageView() const;
        vk::Sampler getVulkanSampler() const;

        void deferredDestroyCurrentResources();

    private:
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        uint32_t mipLevels = 1;

        vk::Image image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        vk::ImageView imageView = VK_NULL_HANDLE;

        // Owned by VulkanSamplerManager
        vk::Sampler sampler = VK_NULL_HANDLE;

        void createVulkanTexture(const unsigned char* data, MipMapFiltering mipMapFiltering);
    };

}
