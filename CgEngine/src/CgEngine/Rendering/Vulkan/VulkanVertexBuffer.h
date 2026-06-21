#pragma once

#include "vk_mem_alloc.h"
#include "Rendering/VertexBuffer.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {

    class VulkanVertexBuffer : public VertexBuffer {
    public:
        explicit VulkanVertexBuffer(size_t size, VertexBufferUsage usage);
        VulkanVertexBuffer(const void* data, size_t size, VertexBufferUsage usage = VertexBufferUsage::Static);

        ~VulkanVertexBuffer();

        VulkanVertexBuffer(VulkanVertexBuffer&& other) noexcept;
        VulkanVertexBuffer& operator=(VulkanVertexBuffer&& other) noexcept;

        VulkanVertexBuffer(VulkanVertexBuffer& other) = delete;
        VulkanVertexBuffer& operator=(VulkanVertexBuffer& other) = delete;

        void setData(const void* data, size_t size) override;
        void setSubData(size_t offset, const void* data, size_t size) override;
        void setLayout(VertexBufferLayout layout) override;
        void setLayout(std::vector<VertexBufferElement> elements) override;

        const VertexBufferLayout& getLayout() const override;

        const vk::Buffer& getVulkanBufferHandle() const;
        size_t getOffsetForCurrentFrame() const;

    private:
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        vk::DeviceSize perFrameSize = 0;
        uint32_t lastWrittenFrameIndex = 0;

        VertexBufferLayout layout;
        VertexBufferUsage usage;
    };

}
