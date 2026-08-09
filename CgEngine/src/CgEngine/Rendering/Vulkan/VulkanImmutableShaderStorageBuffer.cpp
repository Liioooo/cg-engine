#include "VulkanImmutableShaderStorageBuffer.h"
#include "Asserts.h"
#include "VulkanHelpers.h"
#include "Rendering/Renderer.h"
#include "VulkanRenderer.h"

namespace CgEngine {
    VulkanImmutableShaderStorageBuffer::VulkanImmutableShaderStorageBuffer(size_t size, const void *data) : size(size) {
        CG_ASSERT(data != nullptr, "Data pointer cannot be null for VulkanImmutableShaderStorageBuffer")

        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        vk::DeviceSize deviceSize = size;

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

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
        vmaSetAllocationName(allocator, stagingAllocation, "VulkanImmutableShaderStorageBuffer_Staging");

        std::memcpy(stagingAllocDetails.pMappedData, data, size);

        VkBuffer rawBuffer;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = deviceSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer;

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
        vmaSetAllocationName(allocator, allocation, "VulkanImmutableShaderStorageBuffer");

        buffer = rawBuffer;

        VulkanHelpers::copyBuffer(stagingBuffer, buffer, deviceSize);
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
    }

    VulkanImmutableShaderStorageBuffer::~VulkanImmutableShaderStorageBuffer() {
        deferredDestroyCurrentBuffer();
    }

    VulkanImmutableShaderStorageBuffer::VulkanImmutableShaderStorageBuffer(VulkanImmutableShaderStorageBuffer &&other) noexcept : ImmutableShaderStorageBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;
        size = other.size;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanImmutableShaderStorageBuffer& VulkanImmutableShaderStorageBuffer::operator=(VulkanImmutableShaderStorageBuffer &&other) noexcept {
        if (this != &other) {
            ImmutableShaderStorageBuffer::operator=(std::move(other));

            deferredDestroyCurrentBuffer();

            buffer = other.buffer;
            allocation = other.allocation;
            size = other.size;

            other.buffer = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }
        return *this;
    }

    bool VulkanImmutableShaderStorageBuffer::isReady() const {
        return buffer != VK_NULL_HANDLE;
    }

    size_t VulkanImmutableShaderStorageBuffer::getSize() const {
        CG_ASSERT(isReady(), "Shader Storage Buffer is not ready!")
        return size;
    }

    vk::Buffer VulkanImmutableShaderStorageBuffer::getVulkanBufferHandle() const {
        CG_ASSERT(isReady(), "Shader Storage Buffer is not ready!")
        return buffer;
    }

    void VulkanImmutableShaderStorageBuffer::deferredDestroyCurrentBuffer() {
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
