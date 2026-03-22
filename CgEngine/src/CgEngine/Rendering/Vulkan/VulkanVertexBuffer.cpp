#include <Asserts.h>
#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"
#include "VulkanHelpers.h"

namespace CgEngine {
    VulkanVertexBuffer::VulkanVertexBuffer(size_t size, VertexBufferUsage usage) : VulkanVertexBuffer(nullptr, size, usage) {
        CG_ASSERT(usage == VertexBufferUsage::Dynamic, "Static VertexBuffer must be created with data")
    }

    VulkanVertexBuffer::VulkanVertexBuffer(const void* data, size_t size, VertexBufferUsage usage) : usage(usage) {
        CG_ASSERT(size != 0, "VertexBuffer size must be greater than 0")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        if (usage == VertexBufferUsage::Static) {
            CG_ASSERT(data != nullptr, "Static VertexBuffer must be created with data")

            perFrameSize = size;
            vk::DeviceSize bufferSize = size;

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

            memcpy(stagingAllocDetails.pMappedData, data, size);

            VkBuffer rawBuffer;

            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = bufferSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

            VmaAllocationCreateInfo bufferAllocInfo{};
            bufferAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

            vmaCreateBuffer(
                allocator,
                reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo),
                &bufferAllocInfo,
                &rawBuffer,
                &allocation,
                nullptr
            );

            buffer = rawBuffer;
            VulkanHelpers::copyBuffer(stagingBuffer, buffer, bufferSize);
            vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
        } else {
            uint32_t maxFramesInFlight = Renderer::getVulkanBackend()->getMaxFramesInFlight();

            perFrameSize = size;
            vk::DeviceSize totalSize = size * maxFramesInFlight;

            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = totalSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VmaAllocationInfo vmaInfo{};

            VkBuffer rawBuffer;

            vmaCreateBuffer(
                allocator,
                reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo),
                &allocInfo,
                &rawBuffer,
                &allocation,
                &vmaInfo
            );

            buffer = rawBuffer;
            mappedPtr = static_cast<uint8_t*>(vmaInfo.pMappedData);

            if (data) {
                std::memcpy(mappedPtr, data, perFrameSize);
            }
        }
    }

    VulkanVertexBuffer::~VulkanVertexBuffer() {
        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        vmaDestroyBuffer(allocator, buffer, allocation);

        mappedPtr = nullptr;
    }

    VulkanVertexBuffer::VulkanVertexBuffer(VulkanVertexBuffer &&other) noexcept : VertexBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;
        perFrameSize = other.perFrameSize;
        mappedPtr = other.mappedPtr;
        layout = other.layout;
        usage = other.usage;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.mappedPtr = nullptr;
    }

    VulkanVertexBuffer & VulkanVertexBuffer::operator=(VulkanVertexBuffer &&other) noexcept {
        if (this != &other) {
            VertexBuffer::operator=(std::move(other));

            if (buffer != VK_NULL_HANDLE) {
                VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
                vmaDestroyBuffer(allocator, buffer, allocation);
            }

            buffer = other.buffer;
            allocation = other.allocation;
            perFrameSize = other.perFrameSize;
            mappedPtr = other.mappedPtr;
            layout = other.layout;
            usage = other.usage;

            other.buffer = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
            other.mappedPtr = nullptr;
        }
        return *this;
    }

    void VulkanVertexBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(usage == VertexBufferUsage::Dynamic, "setData only valid for dynamic buffers");
        CG_ASSERT(mappedPtr != nullptr, "Buffer not mapped");
        CG_ASSERT(size <= perFrameSize, "Data size exceeds the per-frame buffer size")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t offset = frameIndex * perFrameSize;

        std::memcpy(mappedPtr + offset, data, size);
    }

    void VulkanVertexBuffer::setSubData(size_t offset, const void *data, size_t size) {
        CG_ASSERT(usage == VertexBufferUsage::Dynamic, "setData only valid for dynamic buffers");
        CG_ASSERT(mappedPtr != nullptr, "Buffer not mapped");
        CG_ASSERT(offset + size <= perFrameSize, "Data size and offset exceed the per-frame buffer size")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t bufferOffset = frameIndex * perFrameSize + offset;

        std::memcpy(mappedPtr + bufferOffset, data, size);
    }

    void VulkanVertexBuffer::setLayout(VertexBufferLayout layout) {
        this->layout = layout;
    }

    void VulkanVertexBuffer::setLayout(std::vector<VertexBufferElement> elements) {
        layout = VertexBufferLayout(elements);
    }

    const VertexBufferLayout& VulkanVertexBuffer::getLayout() const {
        return layout;
    }

    const vk::Buffer& VulkanVertexBuffer::getVulkanBufferHandle() const {
        return buffer;
    }

    size_t VulkanVertexBuffer::getOffsetForCurrentFrame() const {
        if (usage == VertexBufferUsage::Dynamic) {
            uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
            return frameIndex * perFrameSize;
        }

        return 0;
    }
}
