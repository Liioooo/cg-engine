#pragma once

#include "Rendering/IndexBuffer.h"
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {

    class VulkanIndexBuffer : public IndexBuffer {
    public:
        VulkanIndexBuffer() = default;
        VulkanIndexBuffer(const void* indices, uint32_t indexCount, IndexBufferDataType type = IndexBufferDataType::UInt32);

        ~VulkanIndexBuffer() override;

        VulkanIndexBuffer(VulkanIndexBuffer&& other) noexcept;
        VulkanIndexBuffer& operator=(VulkanIndexBuffer&& other) noexcept;

        VulkanIndexBuffer(VulkanIndexBuffer& other) = delete;
        VulkanIndexBuffer& operator=(VulkanIndexBuffer& other) = delete;

        void setData(const void* indices, uint32_t indexCount, IndexBufferDataType type) override;
        bool hasData() const override;

        vk::Buffer getVulkanBufferHandle() const;
        vk::IndexType getVulkanIndexType() const;

    private:
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        void setDataInternal(const void* indices);
    };
}
