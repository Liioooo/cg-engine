#include "VulkanTexture2D.h"
#include "Asserts.h"
#include "Logging.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Helpers.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanTexture2D::VulkanTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void *data, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) : width(width), height(height), format(format) {
        createVulkanTexture(static_cast<const unsigned char*>(data), mipMapFiltering);
        sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(wrap, mipMapFiltering, borderColor);
    }

    VulkanTexture2D::VulkanTexture2D(const std::filesystem::path &path, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        auto loadData = loadTextureDataFromFile(path, srgb);

        if (!loadData.data) {
            return;
        }

        format = loadData.format;
        width = loadData.width;
        height = loadData.height;

        createVulkanTexture(loadData.data, mipMapFiltering);
        sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(wrap, mipMapFiltering, borderColor);
        Helpers::freeImageData(loadData.data);
    }

    VulkanTexture2D::VulkanTexture2D(const unsigned char *buffer, int bufferLen, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        auto loadData = loadTextureDataFromMemory(buffer, bufferLen, srgb);

        if (!loadData.data) {
            return;
        }

        format = loadData.format;
        width = loadData.width;
        height = loadData.height;

        createVulkanTexture(loadData.data, mipMapFiltering);
        sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(wrap, mipMapFiltering, borderColor);
        Helpers::freeImageData(loadData.data);
    }

    VulkanTexture2D::~VulkanTexture2D() {
        auto device = Renderer::getVulkanBackend()->getVkDevice();

        if (imageView) {
            device.destroyImageView(imageView);
        }
        if (image) {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyImage(allocator, image, allocation);
        }
    }

    VulkanTexture2D::VulkanTexture2D(VulkanTexture2D &&other) noexcept : Texture2D(std::move(other)) {
        image = other.image;
        allocation = other.allocation;
        imageView = other.imageView;
        sampler = other.sampler;
        mipLevels = other.mipLevels;
        width = other.width;
        height = other.height;
        format = other.format;

        other.image = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.sampler = VK_NULL_HANDLE;
    }

    VulkanTexture2D & VulkanTexture2D::operator=(VulkanTexture2D &&other) noexcept {
        if (this != &other) {
            Texture2D::operator=(std::move(other));

            auto device = Renderer::getVulkanBackend()->getVkDevice();

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
            sampler = other.sampler;
            mipLevels = other.mipLevels;
            width = other.width;
            height = other.height;
            format = other.format;

            other.image = VK_NULL_HANDLE;
            other.imageView = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
            other.sampler = VK_NULL_HANDLE;
        }
        return *this;
    }

    uint32_t VulkanTexture2D::getWidth() const {
        return width;
    }

    uint32_t VulkanTexture2D::getHeight() const {
        return height;
    }

    TextureFormat VulkanTexture2D::getFormat() const {
        return format;
    }

    vk::ImageView VulkanTexture2D::getVulkanImageView() const {
        CG_ASSERT(imageView != VK_NULL_HANDLE, "VulkanTexture2D::getVulkanImageView: Texture has not been created properly.")
        return imageView;
    }

    vk::Sampler VulkanTexture2D::getVulkanSampler() const {
        CG_ASSERT(sampler != VK_NULL_HANDLE, "VulkanTexture2D::getVulkanSampler: Texture has not been created properly.")
        return sampler;
    }

    void VulkanTexture2D::createVulkanTexture(const unsigned char *data, MipMapFiltering mipMapFiltering) {
        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        auto physicalDevice = Renderer::getVulkanBackend()->getVkPhysicalDevice();

        vk::Format vulkanFormat = VulkanHelpers::textureFormatToVulkanFormat(format);
        vk::DeviceSize bufferSize = width * height * Helpers::getBytesPerPixelForTextureFormat(format);

        mipLevels = 1;
        if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
            mipLevels = Helpers::calculateMipCount(width, height);

            vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(vulkanFormat);
            if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
                CG_LOGGING_ERROR("VulkanTexture2D: Format does not support linear blitting, mip maps will not be generated!")
                mipLevels = 1;
            }
        }

        // Staging buffer
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        vk::BufferCreateInfo stagingInfo{};
        stagingInfo.size = bufferSize;
        stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo stagingAllocDetails{};

        vmaCreateBuffer(
            allocator,
            reinterpret_cast<const VkBufferCreateInfo*>(&stagingInfo),
            &stagingAllocInfo,
            &stagingBuffer,
            &stagingAllocation,
            &stagingAllocDetails
        );

        std::memcpy(stagingAllocDetails.pMappedData, data, bufferSize);

        // Device local image
        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = vulkanFormat;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkImage rawImage{};
        vmaCreateImage(
            allocator,
            reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo),
            &imageAllocInfo,
            &rawImage,
            &allocation,
            nullptr
        );

        image = rawImage;

        constexpr vk::PipelineStageFlags2 shaderReadStages = vk::PipelineStageFlagBits2::eAllGraphics | vk::PipelineStageFlagBits2::eComputeShader;

        Renderer::getVulkanBackend()->executeImmediateCommand([&](const vk::CommandBuffer commandBuffer) {
            // Whole mip chain: undefined -> transfer dst, ready to receive the base level copy (and blit writes for the rest)
            vk::ImageMemoryBarrier2 toTransferDst{};
            toTransferDst.oldLayout = vk::ImageLayout::eUndefined;
            toTransferDst.newLayout = vk::ImageLayout::eTransferDstOptimal;
            toTransferDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferDst.image = image;
            toTransferDst.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            toTransferDst.subresourceRange.baseMipLevel = 0;
            toTransferDst.subresourceRange.levelCount = mipLevels;
            toTransferDst.subresourceRange.baseArrayLayer = 0;
            toTransferDst.subresourceRange.layerCount = 1;
            toTransferDst.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
            toTransferDst.srcAccessMask = {};
            toTransferDst.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
            toTransferDst.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;

            vk::DependencyInfo toTransferDstDepInfo{};
            toTransferDstDepInfo.setImageMemoryBarriers(toTransferDst);
            commandBuffer.pipelineBarrier2(toTransferDstDepInfo);

            vk::BufferImageCopy2 copyRegion{};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;
            copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageOffset = vk::Offset3D{0, 0, 0};
            copyRegion.imageExtent = vk::Extent3D{width, height, 1};

            vk::CopyBufferToImageInfo2 copyInfo{};
            copyInfo.srcBuffer = stagingBuffer;
            copyInfo.dstImage = image;
            copyInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
            copyInfo.setRegions(copyRegion);

            commandBuffer.copyBufferToImage2(copyInfo);

            if (mipLevels == 1) {
                vk::ImageMemoryBarrier2 toShaderRead{};
                toShaderRead.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                toShaderRead.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toShaderRead.image = image;
                toShaderRead.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                toShaderRead.subresourceRange.baseMipLevel = 0;
                toShaderRead.subresourceRange.levelCount = 1;
                toShaderRead.subresourceRange.baseArrayLayer = 0;
                toShaderRead.subresourceRange.layerCount = 1;
                toShaderRead.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
                toShaderRead.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
                toShaderRead.dstStageMask = shaderReadStages;
                toShaderRead.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

                vk::DependencyInfo depInfo{};
                depInfo.setImageMemoryBarriers(toShaderRead);
                commandBuffer.pipelineBarrier2(depInfo);

                return;
            }

            int32_t mipWidth = static_cast<int32_t>(width);
            int32_t mipHeight = static_cast<int32_t>(height);

            for (uint32_t i = 1; i < mipLevels; i++) {
                vk::ImageMemoryBarrier2 toTransferSrc{};
                toTransferSrc.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                toTransferSrc.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                toTransferSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toTransferSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                toTransferSrc.image = image;
                toTransferSrc.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                toTransferSrc.subresourceRange.baseMipLevel = i - 1;
                toTransferSrc.subresourceRange.levelCount = 1;
                toTransferSrc.subresourceRange.baseArrayLayer = 0;
                toTransferSrc.subresourceRange.layerCount = 1;
                toTransferSrc.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
                toTransferSrc.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
                toTransferSrc.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
                toTransferSrc.dstAccessMask = vk::AccessFlagBits2::eTransferRead;

                vk::DependencyInfo toTransferSrcDepInfo{};
                toTransferSrcDepInfo.setImageMemoryBarriers(toTransferSrc);
                commandBuffer.pipelineBarrier2(toTransferSrcDepInfo);

                int32_t nextMipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
                int32_t nextMipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

                vk::ImageBlit2 blit{};
                blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
                blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
                blit.dstOffsets[1] = vk::Offset3D{nextMipWidth, nextMipHeight, 1};
                blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vk::BlitImageInfo2 blitInfo{};
                blitInfo.srcImage = image;
                blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
                blitInfo.dstImage = image;
                blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
                blitInfo.filter = vk::Filter::eLinear;
                blitInfo.setRegions(blit);

                commandBuffer.blitImage2(blitInfo);

                vk::ImageMemoryBarrier2 prevMipToShaderRead{};
                prevMipToShaderRead.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                prevMipToShaderRead.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                prevMipToShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                prevMipToShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                prevMipToShaderRead.image = image;
                prevMipToShaderRead.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                prevMipToShaderRead.subresourceRange.baseMipLevel = i - 1;
                prevMipToShaderRead.subresourceRange.levelCount = 1;
                prevMipToShaderRead.subresourceRange.baseArrayLayer = 0;
                prevMipToShaderRead.subresourceRange.layerCount = 1;
                prevMipToShaderRead.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
                prevMipToShaderRead.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
                prevMipToShaderRead.dstStageMask = shaderReadStages;
                prevMipToShaderRead.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

                vk::DependencyInfo prevMipToShaderReadDepInfo{};
                prevMipToShaderReadDepInfo.setImageMemoryBarriers(prevMipToShaderRead);
                commandBuffer.pipelineBarrier2(prevMipToShaderReadDepInfo);

                mipWidth = nextMipWidth;
                mipHeight = nextMipHeight;
            }

            // The last mip level was only ever written to (as a blit destination), so it still needs a transition of its own.
            vk::ImageMemoryBarrier2 lastMipToShaderRead{};
            lastMipToShaderRead.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            lastMipToShaderRead.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            lastMipToShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            lastMipToShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            lastMipToShaderRead.image = image;
            lastMipToShaderRead.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            lastMipToShaderRead.subresourceRange.baseMipLevel = mipLevels - 1;
            lastMipToShaderRead.subresourceRange.levelCount = 1;
            lastMipToShaderRead.subresourceRange.baseArrayLayer = 0;
            lastMipToShaderRead.subresourceRange.layerCount = 1;
            lastMipToShaderRead.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
            lastMipToShaderRead.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
            lastMipToShaderRead.dstStageMask = shaderReadStages;
            lastMipToShaderRead.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

            vk::DependencyInfo lastMipToShaderReadDepInfo{};
            lastMipToShaderReadDepInfo.setImageMemoryBarriers(lastMipToShaderRead);
            commandBuffer.pipelineBarrier2(lastMipToShaderReadDepInfo);
        });

        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

        imageView = VulkanHelpers::createImageView2D(image, vulkanFormat, mipLevels, 1, vk::ImageAspectFlagBits::eColor);
    }
}