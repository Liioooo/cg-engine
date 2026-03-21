#pragma once

#include <vulkan/vulkan.hpp>

namespace CgEngine {

    class VulkanDescriptorAllocator {
    public:
        VulkanDescriptorAllocator() = default;

        void init(vk::Device device);
        void shutdown();

        std::vector<vk::DescriptorSet> allocateDescriptorSets(vk::DescriptorSetLayout layout, uint32_t count);
        vk::DescriptorSet allocateDescriptorSet(vk::DescriptorSetLayout layout);

    private:
        static constexpr uint32_t BASE_POOL_SIZE = 128;

        struct DescriptorPool {
            vk::DescriptorPool vulkanDescriptorPool;
            uint32_t setsAllocated = 0;
        };

        std::vector<DescriptorPool> descriptorPools;
        DescriptorPool* currentDescriptorPool = nullptr;

        vk::Device vkDevice;

        DescriptorPool createDescriptorPool(uint32_t setCount);
    };

}