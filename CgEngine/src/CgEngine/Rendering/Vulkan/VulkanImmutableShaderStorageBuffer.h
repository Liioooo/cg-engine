#pragma once

#include "Rendering/ImmutableShaderStorageBuffer.h"
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>


namespace CgEngine {
    class VulkanImmutableShaderStorageBuffer : public ImmutableShaderStorageBuffer {
    public:
        VulkanImmutableShaderStorageBuffer() = default;
        VulkanImmutableShaderStorageBuffer(size_t size, const void* data);

        ~VulkanImmutableShaderStorageBuffer() override;

        VulkanImmutableShaderStorageBuffer(VulkanImmutableShaderStorageBuffer&& other) noexcept;
        VulkanImmutableShaderStorageBuffer& operator=(VulkanImmutableShaderStorageBuffer&& other) noexcept;

        VulkanImmutableShaderStorageBuffer(VulkanImmutableShaderStorageBuffer& other) = delete;
        VulkanImmutableShaderStorageBuffer& operator=(VulkanImmutableShaderStorageBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        vk::Buffer getVulkanBufferHandle() const;

        void deferredDestroyCurrentBuffer();

    private:
        vk::Buffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        size_t size = 0;
    };
}