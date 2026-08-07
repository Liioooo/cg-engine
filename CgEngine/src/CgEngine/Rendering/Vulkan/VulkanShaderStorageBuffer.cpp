#include "VulkanShaderStorageBuffer.h"

#include "Asserts.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(size_t size) : VulkanShaderStorageBuffer(size, nullptr) {}

    VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(size_t size, const void* data) : perFrameSize(size) {
        CG_ASSERT(size != 0, "ShaderStorageBuffer size must be greater than 0")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        uint32_t maxFramesInFlight = Renderer::getVulkanBackend()->getMaxFramesInFlight();

        vk::PhysicalDeviceProperties props = Renderer::getVulkanBackend()->getVkPhysicalDevice().getProperties();

        size_t alignment = props.limits.minStorageBufferOffsetAlignment;
        alignedFrameSize = VulkanHelpers::alignUp(size, alignment);

        vk::DeviceSize totalSize = alignedFrameSize * maxFramesInFlight;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = totalSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo allocDetails{};

        VkBuffer rawBuffer{};
        vmaCreateBuffer(
            allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
            &allocInfo,
            &rawBuffer,
            &allocation,
            &allocDetails
        );

        buffer = rawBuffer;

        if (data != nullptr) {
            uint8_t* mappedPtr = static_cast<uint8_t*>(allocDetails.pMappedData);
            for (uint32_t i = 0; i < maxFramesInFlight; i++) {
                std::memcpy(mappedPtr + alignedFrameSize * i, data, size);
            }
        }
    }

    VulkanShaderStorageBuffer::~VulkanShaderStorageBuffer() {
        if (buffer != VK_NULL_HANDLE) {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyBuffer(allocator, buffer, allocation);
            buffer = VK_NULL_HANDLE;
            allocation = VK_NULL_HANDLE;
        }
    }

    VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(VulkanShaderStorageBuffer &&other) noexcept : ShaderStorageBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;
        perFrameSize = other.perFrameSize;
        alignedFrameSize = other.alignedFrameSize;
        lastWrittenFrameIndex = other.lastWrittenFrameIndex;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanShaderStorageBuffer& VulkanShaderStorageBuffer::operator=(VulkanShaderStorageBuffer &&other) noexcept {
        if (this != &other) {
            ShaderStorageBuffer::operator=(std::move(other));

            if (buffer != VK_NULL_HANDLE) {
                VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
                vmaDestroyBuffer(allocator, buffer, allocation);
            }

            buffer = other.buffer;
            allocation = other.allocation;
            perFrameSize = other.perFrameSize;
            alignedFrameSize = other.alignedFrameSize;
            lastWrittenFrameIndex = other.lastWrittenFrameIndex;

            other.buffer = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }
        return *this;
    }

    bool VulkanShaderStorageBuffer::isReady() const {
        return buffer != VK_NULL_HANDLE;
    }

    size_t VulkanShaderStorageBuffer::getSize() const {
        CG_ASSERT(isReady(), "VulkanShaderStorageBuffer is not ready")
        return perFrameSize;
    }

    void VulkanShaderStorageBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(isReady(), "VulkanShaderStorageBuffer is not ready")
        CG_ASSERT(size == this->perFrameSize, "VulkanShaderStorageBuffer::setData: Data size does not match the ShaderStorageBuffer size.")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t offset = frameIndex * alignedFrameSize;

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, allocation, &info);
        uint8_t* mappedPtr = static_cast<uint8_t*>(info.pMappedData);

        std::memcpy(mappedPtr + offset, data, size);

        lastWrittenFrameIndex = frameIndex;
    }

    void VulkanShaderStorageBuffer::setSubData(size_t offset, const void* data, size_t size) {
        CG_ASSERT(isReady(), "VulkanShaderStorageBuffer is not ready")
        CG_ASSERT(offset + size <= this->perFrameSize, "VulkanShaderStorageBuffer::setSubData: Data size and offset exceed the buffer size.")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t bufferOffset = frameIndex * alignedFrameSize + offset;

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, allocation, &info);
        uint8_t* mappedPtr = static_cast<uint8_t*>(info.pMappedData);

        std::memcpy(mappedPtr + bufferOffset, data, size);

        lastWrittenFrameIndex = frameIndex;
    }

    size_t VulkanShaderStorageBuffer::getAlignedFrameSize() const {
        return alignedFrameSize;
    }

    vk::Buffer VulkanShaderStorageBuffer::getVulkanBufferHandle() const {
        return buffer;
    }

    size_t VulkanShaderStorageBuffer::getOffsetForCurrentFrame() const {
        return alignedFrameSize * lastWrittenFrameIndex;
    }

}