#pragma once
#include "Rendering/UniformBuffer.h"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace CgEngine {
    class VulkanUniformBuffer : public UniformBuffer {
    public:
        VulkanUniformBuffer() = default;
        explicit VulkanUniformBuffer(size_t size);

        ~VulkanUniformBuffer() override;

        VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept;
        VulkanUniformBuffer& operator=(VulkanUniformBuffer&& other) noexcept;

        VulkanUniformBuffer(VulkanUniformBuffer& other) = delete;
        VulkanUniformBuffer& operator=(VulkanUniformBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        void setData(const void* data, size_t size) override;

        size_t getAlignedFrameSize() const;
        vk::Buffer getVulkanBufferHandle() const;
        size_t getOffsetForCurrentFrame() const;

    private:
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        size_t perFrameSize = 0;
        size_t alignedFrameSize = 0;
        uint32_t lastWrittenFrameIndex = 0;

    };
}
