#pragma once

#include "Rendering/ShaderStorageBuffer.h"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace CgEngine {

    class VulkanShaderStorageBuffer : public ShaderStorageBuffer {
    public:
        VulkanShaderStorageBuffer() = default;
        explicit VulkanShaderStorageBuffer(size_t size);
        VulkanShaderStorageBuffer(size_t size, const void* data);

        ~VulkanShaderStorageBuffer() override;

        VulkanShaderStorageBuffer(VulkanShaderStorageBuffer&& other) noexcept;
        VulkanShaderStorageBuffer& operator=(VulkanShaderStorageBuffer&& other) noexcept;

        VulkanShaderStorageBuffer(VulkanShaderStorageBuffer& other) = delete;
        VulkanShaderStorageBuffer& operator=(VulkanShaderStorageBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        void setData(const void* data, size_t size) override;
        void setSubData(size_t offset, const void* data, size_t size) override;

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