#include "VulkanUniformBuffer.h"

#include "Asserts.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanUniformBuffer::VulkanUniformBuffer(size_t size) : perFrameSize(size) {
        CG_ASSERT(size != 0, "UniformBuffer size must be greater than 0")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        int32_t maxFramesInFlight = Renderer::getVulkanBackend()->getMaxFramesInFlight();

        vk::PhysicalDeviceProperties props = Renderer::getVulkanBackend()->getVkPhysicalDevice().getProperties();

        size_t alignment = props.limits.minUniformBufferOffsetAlignment;
        alignedFrameSize = VulkanHelpers::alignUp(size, alignment);

        vk::DeviceSize totalSize = alignedFrameSize * maxFramesInFlight;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = totalSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer rawBuffer{};
        vmaCreateBuffer(
            allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
            &allocInfo,
            &rawBuffer,
            &allocation,
            nullptr
        );
        vmaSetAllocationName(allocator, allocation, "VulkanUniformBuffer");

        buffer = rawBuffer;
    }

    VulkanUniformBuffer::~VulkanUniformBuffer() {
        deferredDestroyCurrentBuffer();
    }

    VulkanUniformBuffer::VulkanUniformBuffer(VulkanUniformBuffer &&other) noexcept : UniformBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;
        perFrameSize = other.perFrameSize;
        alignedFrameSize = other.alignedFrameSize;
        lastWrittenFrameIndex = other.lastWrittenFrameIndex;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanUniformBuffer & VulkanUniformBuffer::operator=(VulkanUniformBuffer &&other) noexcept {
        if (this != &other) {
            UniformBuffer::operator=(std::move(other));

            deferredDestroyCurrentBuffer();

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

    bool VulkanUniformBuffer::isReady() const {
        return buffer != VK_NULL_HANDLE;
    }

    size_t VulkanUniformBuffer::getSize() const {
        CG_ASSERT(isReady(), "VulkanUniformBuffer is not ready")
        return perFrameSize;
    }

    void VulkanUniformBuffer::setData(const void *data, size_t size) {
        CG_ASSERT(isReady(), "VulkanUniformBuffer is not ready")
        CG_ASSERT(size == this->perFrameSize, "VulkanUniformBuffer::setData: Data size does not match the UniformBuffer size.")

        uint32_t frameIndex = Renderer::getVulkanBackend()->getCurrentFrameIndex();
        size_t offset = frameIndex * alignedFrameSize;

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, allocation, &info);
        uint8_t* mappedPtr = static_cast<uint8_t*>(info.pMappedData);

        std::memcpy(mappedPtr + offset, data, size);

        lastWrittenFrameIndex = frameIndex;
    }

    size_t VulkanUniformBuffer::getAlignedFrameSize() const {
        return alignedFrameSize;
    }

    vk::Buffer VulkanUniformBuffer::getVulkanBufferHandle() const {
        return buffer;
    }

    size_t VulkanUniformBuffer::getOffsetForCurrentFrame() const {
        return alignedFrameSize * lastWrittenFrameIndex;
    }

    void VulkanUniformBuffer::deferredDestroyCurrentBuffer() {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }

        vk::Buffer oldBuffer = buffer;
        VmaAllocation oldAllocation = allocation;

        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;

        Renderer::getVulkanBackend()->deferDestruction([oldBuffer, oldAllocation] {
            VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();
            vmaDestroyBuffer(allocator, oldBuffer, oldAllocation);
        });
    }
}
