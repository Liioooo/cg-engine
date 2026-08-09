#include "VulkanTextureCube.h"
#include "Asserts.h"
#include "Logging.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Helpers.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    constexpr vk::PipelineStageFlags2 shaderReadStages = vk::PipelineStageFlagBits2::eAllGraphics | vk::PipelineStageFlagBits2::eComputeShader;

    VulkanTextureCube::VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering) : width(width), height(height), format(format) {
        createVulkanTextureCube(nullptr, mipMapFiltering);
        sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(TextureWrap::Clamp, mipMapFiltering, TextureBorderColor::OpaqueWhite);
    }

    VulkanTextureCube::VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void *data, MipMapFiltering mipMapFiltering) : width(width), height(height), format(format) {
        createVulkanTextureCube(data, mipMapFiltering);
        sampler = Renderer::getVulkanBackend()->getSamplerManager().getSampler(TextureWrap::Clamp, mipMapFiltering, TextureBorderColor::OpaqueWhite);
    }

    VulkanTextureCube::~VulkanTextureCube() {
        deferredDestroyCurrentResources();
    }

    VulkanTextureCube::VulkanTextureCube(VulkanTextureCube &&other) noexcept : TextureCube(std::move(other)) {
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

    VulkanTextureCube & VulkanTextureCube::operator=(VulkanTextureCube &&other) noexcept {
        if (this != &other) {
            TextureCube::operator=(std::move(other));

            deferredDestroyCurrentResources();

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

    bool VulkanTextureCube::isReady() const {
        return image != VK_NULL_HANDLE;
    }

    uint32_t VulkanTextureCube::getWidth() const {
        return width;
    }

    uint32_t VulkanTextureCube::getHeight() const {
        return height;
    }

    TextureFormat VulkanTextureCube::getFormat() const {
        return format;
    }

    void VulkanTextureCube::generateMipMaps() {
        CG_ASSERT(isReady(), "VulkanTextureCube::generateMipMaps: Texture is not ready!")

        if (mipLevels == 1) {
            return;
        }

        Renderer::getVulkanBackend()->executeImmediateCommand([this](const vk::CommandBuffer commandBuffer) {
            recordLayoutTransition(commandBuffer, 0, mipLevels, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal, shaderReadStages, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite);
            recordMipMapGeneration(commandBuffer);
        });
    }

    vk::ImageView VulkanTextureCube::getVulkanImageView() const {
        CG_ASSERT(imageView != VK_NULL_HANDLE, "VulkanTextureCube::getVulkanImageView: Texture has not been created properly.")
        return imageView;
    }

    vk::Sampler VulkanTextureCube::getVulkanSampler() const {
        CG_ASSERT(sampler != VK_NULL_HANDLE, "VulkanTextureCube::getVulkanSampler: Texture has not been created properly.")
        return sampler;
    }

    vk::ImageView VulkanTextureCube::createStorageImageView(uint32_t mipLevel) const {
        CG_ASSERT(isReady(), "VulkanTextureCube::createStorageImageView: Texture has not been created properly.")
        return VulkanHelpers::createImageViewCube(image, VulkanHelpers::textureFormatToVulkanFormat(format), 1, vk::ImageAspectFlagBits::eColor, mipLevel);
    }

    void VulkanTextureCube::createVulkanTextureCube(const void *data, MipMapFiltering mipMapFiltering) {
        CG_ASSERT(width == height, "VulkanTextureCube: Cube map faces have to be square!")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        auto physicalDevice = Renderer::getVulkanBackend()->getVkPhysicalDevice();

        vk::Format vulkanFormat = VulkanHelpers::textureFormatToVulkanFormat(format);
        vk::DeviceSize faceSize = width * height * Helpers::getBytesPerPixelForTextureFormat(format);

        // OpenGL allocates the mip chain lazily inside glGenerateMipmap, Vulkan cannot - so the whole chain is
        // always allocated here and generateMipMaps() only fills it.
        mipLevels = Helpers::calculateMipCount(width, height);
        if (mipLevels > 1) {
            vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(vulkanFormat);
            if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
                CG_LOGGING_ERROR("VulkanTextureCube: Format does not support linear blitting, mip maps will not be available!")
                mipLevels = 1;
            }
        }

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = vulkanFormat;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = FACE_COUNT;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage;

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
        vmaSetAllocationName(allocator, allocation, "VulkanTextureCube");

        image = rawImage;
        imageView = VulkanHelpers::createImageViewCube(image, vulkanFormat, mipLevels, vk::ImageAspectFlagBits::eColor);

        if (data == nullptr) {
            // Nothing to upload, the image just has to leave the undefined layout to be usable
            Renderer::getVulkanBackend()->executeImmediateCommand([this](const vk::CommandBuffer commandBuffer) {
                recordLayoutTransition(commandBuffer, 0, mipLevels, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, {}, shaderReadStages, vk::AccessFlagBits2::eShaderRead);
            });

            return;
        }

        // Staging buffer, holds the data of a single face - like OpenGLTextureCube, every face is filled with the same data
        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        vk::BufferCreateInfo stagingInfo{};
        stagingInfo.size = faceSize;
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
        vmaSetAllocationName(allocator, stagingAllocation, "VulkanTextureCube_Staging");

        std::memcpy(stagingAllocDetails.pMappedData, data, faceSize);

        bool withMipMaps = mipLevels > 1 && (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic);

        Renderer::getVulkanBackend()->executeImmediateCommand([&](const vk::CommandBuffer commandBuffer) {
            // Whole mip chain: undefined -> transfer dst, ready to receive the base level copies (and blit writes for the rest)
            recordLayoutTransition(commandBuffer, 0, mipLevels, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, {}, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite);

            std::array<vk::BufferImageCopy2, FACE_COUNT> copyRegions{};
            for (uint32_t face = 0; face < FACE_COUNT; face++) {
                vk::BufferImageCopy2& copyRegion = copyRegions[face];
                copyRegion.bufferOffset = 0;
                copyRegion.bufferRowLength = 0;
                copyRegion.bufferImageHeight = 0;
                copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                copyRegion.imageSubresource.mipLevel = 0;
                copyRegion.imageSubresource.baseArrayLayer = face;
                copyRegion.imageSubresource.layerCount = 1;
                copyRegion.imageOffset = vk::Offset3D{0, 0, 0};
                copyRegion.imageExtent = vk::Extent3D{width, height, 1};
            }

            vk::CopyBufferToImageInfo2 copyInfo{};
            copyInfo.srcBuffer = stagingBuffer;
            copyInfo.dstImage = image;
            copyInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
            copyInfo.setRegions(copyRegions);

            commandBuffer.copyBufferToImage2(copyInfo);

            if (withMipMaps) {
                recordMipMapGeneration(commandBuffer);
            } else {
                recordLayoutTransition(commandBuffer, 0, mipLevels, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, shaderReadStages, vk::AccessFlagBits2::eShaderRead);
            }
        });

        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
    }

    void VulkanTextureCube::recordLayoutTransition(const vk::CommandBuffer &commandBuffer, uint32_t baseMipLevel, uint32_t levelCount, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStage, vk::AccessFlags2 srcAccess, vk::PipelineStageFlags2 dstStage, vk::AccessFlags2 dstAccess) const {
        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = baseMipLevel;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = FACE_COUNT;
        barrier.srcStageMask = srcStage;
        barrier.srcAccessMask = srcAccess;
        barrier.dstStageMask = dstStage;
        barrier.dstAccessMask = dstAccess;

        vk::DependencyInfo depInfo{};
        depInfo.setImageMemoryBarriers(barrier);
        commandBuffer.pipelineBarrier2(depInfo);
    }

    void VulkanTextureCube::recordMipMapGeneration(const vk::CommandBuffer &commandBuffer) const {
        // Expects every mip level to be in transfer dst layout, leaves all of them in shader read only layout
        int32_t mipWidth = static_cast<int32_t>(width);
        int32_t mipHeight = static_cast<int32_t>(height);

        for (uint32_t i = 1; i < mipLevels; i++) {
            recordLayoutTransition(commandBuffer, i - 1, 1, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead);

            int32_t nextMipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
            int32_t nextMipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

            // All six faces are downsampled with a single blit
            vk::ImageBlit2 blit{};
            blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
            blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = FACE_COUNT;
            blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
            blit.dstOffsets[1] = vk::Offset3D{nextMipWidth, nextMipHeight, 1};
            blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = FACE_COUNT;

            vk::BlitImageInfo2 blitInfo{};
            blitInfo.srcImage = image;
            blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
            blitInfo.dstImage = image;
            blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
            blitInfo.filter = vk::Filter::eLinear;
            blitInfo.setRegions(blit);

            commandBuffer.blitImage2(blitInfo);

            recordLayoutTransition(commandBuffer, i - 1, 1, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead, shaderReadStages, vk::AccessFlagBits2::eShaderRead);

            mipWidth = nextMipWidth;
            mipHeight = nextMipHeight;
        }

        // The last mip level was only ever written to (as a blit destination), so it still needs a transition of its own.
        recordLayoutTransition(commandBuffer, mipLevels - 1, 1, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, shaderReadStages, vk::AccessFlagBits2::eShaderRead);
    }

    void VulkanTextureCube::deferredDestroyCurrentResources() {
        if (!image && !imageView) {
            return;
        }

        vk::Image oldImage = image;
        VmaAllocation oldAllocation = allocation;
        vk::ImageView oldImageView = imageView;

        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        imageView = VK_NULL_HANDLE;

        Renderer::getVulkanBackend()->deferDestruction([oldImage, oldAllocation, oldImageView] {
            auto device = Renderer::getVulkanBackend()->getVkDevice();

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