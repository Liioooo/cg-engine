#include "VulkanDescriptorAllocator.h"

#include "Logging.h"

namespace CgEngine {

    void VulkanDescriptorAllocator::init(vk::Device device) {
        vkDevice = device;

        descriptorPools.clear();
        currentDescriptorPool = nullptr;
    }

    void VulkanDescriptorAllocator::shutdown() {
        for (auto& pool : descriptorPools) {
            if (pool.vulkanDescriptorPool) {
                vkDevice.destroyDescriptorPool(pool.vulkanDescriptorPool);
            }
        }
        descriptorPools.clear();
        currentDescriptorPool = nullptr;
    }

    std::vector<vk::DescriptorSet> VulkanDescriptorAllocator::allocateDescriptorSets(vk::DescriptorSetLayout layout, uint32_t count) {
        if (!currentDescriptorPool) {
            uint32_t size = BASE_POOL_SIZE;
            descriptorPools.emplace_back(createDescriptorPool(size));
            currentDescriptorPool = &descriptorPools.back();
        }

        std::vector<vk::DescriptorSetLayout> layouts(count, layout);

        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.setDescriptorPool(currentDescriptorPool->vulkanDescriptorPool);
        allocInfo.setSetLayouts(layouts);

        auto setsResults = vkDevice.allocateDescriptorSets(allocInfo);

        if (setsResults.result == vk::Result::eSuccess) {
            currentDescriptorPool->setsAllocated += count;
            return setsResults.value;
        }

        if (setsResults.result != vk::Result::eErrorOutOfPoolMemory && setsResults.result != vk::Result::eErrorFragmentedPool) {
            CG_LOGGING_ERROR("Descriptor allocation failed with unexpected error!")
        }

        uint32_t newSize = BASE_POOL_SIZE * (1u << descriptorPools.size());
        descriptorPools.emplace_back(createDescriptorPool(newSize));
        currentDescriptorPool = &descriptorPools.back();

        allocInfo.setDescriptorPool(currentDescriptorPool->vulkanDescriptorPool);

        setsResults = vkDevice.allocateDescriptorSets(allocInfo);

        if (setsResults.result != vk::Result::eSuccess) {
            CG_LOGGING_ERROR("VulkanDescriptorAllocator: Failed to allocate descriptor sets even after creating new pool!")
        }

        currentDescriptorPool->setsAllocated += count;
        return setsResults.value;
    }

    vk::DescriptorSet VulkanDescriptorAllocator::allocateDescriptorSet(vk::DescriptorSetLayout layout) {
        return allocateDescriptorSets(layout, 1)[0];
    }

    VulkanDescriptorAllocator::DescriptorPool VulkanDescriptorAllocator::createDescriptorPool(uint32_t setCount) {
        std::array<vk::DescriptorPoolSize, 3> sizes = {
            vk::DescriptorPoolSize{
                vk::DescriptorType::eUniformBuffer,
                setCount * 8
            },
            vk::DescriptorPoolSize{
                vk::DescriptorType::eStorageBuffer,
                setCount * 8
            },
            vk::DescriptorPoolSize{
                vk::DescriptorType::eCombinedImageSampler,
                setCount * 8
            }
        };

        vk::DescriptorPoolCreateInfo info{};
        info.setMaxSets(setCount);
        info.setPoolSizes(sizes);
        info.setPoolSizeCount(sizes.size());
        info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        DescriptorPool pool{};

        auto poolResult = vkDevice.createDescriptorPool(info);
        if (!poolResult.has_value()) {
            CG_LOGGING_ERROR("VulkanDescriptorAllocator: Failed to create descriptor pool!")
        }

        pool.vulkanDescriptorPool = poolResult.value;
        pool.setsAllocated = 0;

        return pool;
    }

}
