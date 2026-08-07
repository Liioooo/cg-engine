#include <Asserts.h>
#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"
#include "VulkanHelpers.h"

namespace CgEngine {
    VulkanVertexBuffer::VulkanVertexBuffer(size_t size, VertexBufferUsage usage) : VulkanVertexBuffer(nullptr, size, usage) {
        CG_ASSERT(usage != VertexBufferUsage::Static, "Static VertexBuffer must be created with data")
    }

    VulkanVertexBuffer::VulkanVertexBuffer(const void* data, size_t size, VertexBufferUsage usage) : usage(usage) {
        CG_ASSERT(size != 0, "VertexBuffer size must be greater than 0")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        if (usage == VertexBufferUsage::Static) {
            CG_ASSERT(data != nullptr, "Static VertexBuffer must be created with data")
        }

        if (usage == VertexBufferUsage::Static || usage == VertexBufferUsage::GPUDynamic) {
            perFrameSize = size;
            vk::DeviceSize deviceSize = perFrameSize;

            VkBuffer stagingBuffer;
            VmaAllocation stagingAllocation;

            if (data != nullptr) {
                vk::BufferCreateInfo stagingInfo{};
                stagingInfo.size = deviceSize;
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

                std::memcpy(stagingAllocDetails.pMappedData, data, size);
            }

            VkBuffer rawBuffer;

            vk::BufferCreateInfo bufferInfo{};
            bufferInfo.size = deviceSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer;

            if (data != nullptr) {
                bufferInfo.usage |= vk::BufferUsageFlagBits::eTransferDst;
            }

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

            if (data != nullptr) {
                VulkanHelpers::copyBuffer(stagingBuffer, buffer, deviceSize);
                vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
            }
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

            if (data) {
                uint8_t* mappedPtr = static_cast<uint8_t*>(vmaInfo.pMappedData);
                for (uint32_t i = 0; i < maxFramesInFlight; i++) {
                    std::memcpy(mappedPtr + perFrameSize * i, data, perFrameSize);
                }
            }
        }
    }

    VulkanVertexBuffer::~VulkanVertexBuffer() {
        if (buffer != VK_NULL_HANDLE) {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyBuffer(allocator, buffer, allocation);
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }

    VulkanVertexBuffer::VulkanVertexBuffer(VulkanVertexBuffer &&other) noexcept : VertexBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;
        perFrameSize = other.perFrameSize;
        layout = other.layout;
        usage = other.usage;
        lastWrittenFrameIndex = other.lastWrittenFrameIndex;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
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
            layout = other.layout;
            usage = other.usage;
            lastWrittenFrameIndex = other.lastWrittenFrameIndex;

            other.buffer = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }
        return *this;
    }

    void VulkanVertexBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(usage == VertexBufferUsage::CPUDynamic, "Only CPUDynamic vertex buffers can be updated");
        CG_ASSERT(size <= perFrameSize, "Data size exceeds the per-frame buffer size")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t offset = frameIndex * perFrameSize;

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, allocation, &info);
        uint8_t* mappedPtr = static_cast<uint8_t*>(info.pMappedData);

        std::memcpy(mappedPtr + offset, data, size);

        lastWrittenFrameIndex = frameIndex;
    }

    void VulkanVertexBuffer::setSubData(size_t offset, const void *data, size_t size) {
        CG_ASSERT(usage == VertexBufferUsage::CPUDynamic, "Only CPUDynamic vertex buffers can be updated");
        CG_ASSERT(offset + size <= perFrameSize, "Data size and offset exceed the per-frame buffer size")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t bufferOffset = frameIndex * perFrameSize + offset;

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, allocation, &info);
        uint8_t* mappedPtr = static_cast<uint8_t*>(info.pMappedData);

        std::memcpy(mappedPtr + bufferOffset, data, size);

        lastWrittenFrameIndex = frameIndex;
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
        if (usage == VertexBufferUsage::CPUDynamic) {
            return lastWrittenFrameIndex * perFrameSize;
        }

        return 0;
    }
}
