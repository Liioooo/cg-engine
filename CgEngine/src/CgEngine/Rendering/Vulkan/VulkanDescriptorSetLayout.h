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

        bool isReady() const override;

        vk::DescriptorSetLayout getDescriptorSetLayout() const;

    private:
        vk::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    };

}