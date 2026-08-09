#include "VulkanIndexBuffer.h"
#include "Asserts.h"
#include "VulkanHelpers.h"
#include "Rendering/Renderer.h"
#include "VulkanRenderer.h"


namespace CgEngine {
    VulkanIndexBuffer::VulkanIndexBuffer(const void *indices, uint32_t indexCount, IndexBufferDataType type) : IndexBuffer(indexCount, type) {
        setDataInternal(indices);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer() {
        deferredDestroyCurrentBuffer();
    }

    VulkanIndexBuffer::VulkanIndexBuffer(VulkanIndexBuffer &&other) noexcept : IndexBuffer(std::move(other)) {
        buffer = other.buffer;
        allocation = other.allocation;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanIndexBuffer& VulkanIndexBuffer::operator=(VulkanIndexBuffer &&other) noexcept {
        if (this != &other) {
            IndexBuffer::operator=(std::move(other));

            deferredDestroyCurrentBuffer();

            buffer = other.buffer;
            allocation = other.allocation;

            other.buffer = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }
        return *this;
    }

    void VulkanIndexBuffer::setData(const void *indices, uint32_t indexCount, IndexBufferDataType type) {
        CG_ASSERT(!hasData(), "VulkanIndexBuffer::setData: Buffer already has data. You cannot change data of an existing IndexBuffer.")

        this->indexCount = indexCount;
        this->dataType = type;
        setDataInternal(indices);
    }

    bool VulkanIndexBuffer::hasData() const {
        return buffer != VK_NULL_HANDLE;
    }

    vk::Buffer VulkanIndexBuffer::getVulkanBufferHandle() const {
        CG_ASSERT(hasData(), "VulkanIndexBuffer::getVulkanBufferHandle: Buffer has no data.")
        return buffer;
    }

    vk::IndexType VulkanIndexBuffer::getVulkanIndexType() const {
        switch (dataType) {
            case IndexBufferDataType::UInt8:
                return vk::IndexType::eUint8EXT;
            case IndexBufferDataType::UInt16:
                return vk::IndexType::eUint16;
            case IndexBufferDataType::UInt32:
                return vk::IndexType::eUint32;
        }
        return vk::IndexType::eUint32;
    }

    void VulkanIndexBuffer::setDataInternal(const void* indices) {
        VmaAllocator allocator = Renderer::getVulkanBackend()->getVmaAllocator();

        vk::DeviceSize bufferSize = getSizeOfIndexBufferDataType(dataType) * indexCount;

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
        vmaSetAllocationName(allocator, stagingAllocation, "VulkanIndexBuffer_Staging");

        memcpy(stagingAllocDetails.pMappedData, indices, bufferSize);

        VkBuffer rawBuffer;

        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = bufferSize;
        bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;

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
        vmaSetAllocationName(allocator, allocation, "VulkanIndexBuffer");

        buffer = rawBuffer;
        VulkanHelpers::copyBuffer(stagingBuffer, buffer, bufferSize);
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
    }

    void VulkanIndexBuffer::deferredDestroyCurrentBuffer() {
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
