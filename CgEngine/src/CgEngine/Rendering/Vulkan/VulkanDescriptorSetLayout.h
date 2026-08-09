#pragma once
#include "Rendering/DescriptorSetLayout.h"
#include <vulkan/vulkan.hpp>

namespace CgEngine {
    class VulkanDescriptorSetLayout : public DescriptorSetLayout {
    public:
        VulkanDescriptorSetLayout() = default;
        explicit VulkanDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec);

        ~VulkanDescriptorSetLayout() override;

        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
        VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept;

        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout& other) = delete;
        VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout& other) = delete;

        DescriptorSetLayoutBindingUsage getDescriptorSetLayoutBindingUsageForBindingPoint(uint32_t bindingPoint) const override;

        bool isReady() const override;

        vk::DescriptorSetLayout getDescriptorSetLayout() const;

        void deferredDestroyCurrentResources();

    private:
        vk::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, DescriptorSetLayoutBindingUsage> bindingPointUsageMap{};
    };

}