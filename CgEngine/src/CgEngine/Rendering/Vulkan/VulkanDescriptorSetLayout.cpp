#include "VulkanDescriptorSetLayout.h"
#include "Asserts.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayoutSpecification &spec) {
        CG_ASSERT(validateBindingPoints({{spec.uboBindingPoints, "UBO"}, {spec.ssboBindingPoints, "SSBO"}, {spec.texture2DAndAttachmentBindingPoints, "Texture2D/Attachment"}, {spec.imageBindingPoints, "Image"}}), "DescriptorSetLayout: Binding points are overlapping!")

        std::vector<vk::DescriptorSetLayoutBinding> vulkanBindings;
        vulkanBindings.reserve(spec.uboBindingPoints.size() + spec.ssboBindingPoints.size() + spec.texture2DAndAttachmentBindingPoints.size() + spec.imageBindingPoints.size());

        vk::ShaderStageFlags stageFlags;

        switch (spec.usage) {
            case DescriptorSetLayoutUsage::Graphics:
                stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
                break;
            case DescriptorSetLayoutUsage::Compute:
                stageFlags = vk::ShaderStageFlagBits::eCompute;
                break;
            case DescriptorSetLayoutUsage::GraphicsAndCompute:
                stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
                break;
        }

        for (const auto& uboBindingPoint : spec.uboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = uboBindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& ssboBindingPoint : spec.ssboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = ssboBindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& tex2DBindingPoint : spec.texture2DAndAttachmentBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = tex2DBindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& tex2DBindingPoint : spec.imageBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = tex2DBindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        vk::DescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.setBindings(vulkanBindings);

        auto device = Renderer::getVulkanBackend()->getVkDevice();
        auto result = device.createDescriptorSetLayout(layoutInfo);
        if (!result.has_value()) {
            CG_LOGGING_ERROR("VulkanDescriptorSetLayout: Failed to create descriptor set layout!")
        }
        descriptorSetLayout = result.value;
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
        if (descriptorSetLayout != VK_NULL_HANDLE) {
            auto device = Renderer::getVulkanBackend()->getVkDevice();
            device.destroyDescriptorSetLayout(descriptorSetLayout);
        }
    }

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&other) noexcept : DescriptorSetLayout(std::move(other)) {
        descriptorSetLayout = other.descriptorSetLayout;
        other.descriptorSetLayout = VK_NULL_HANDLE;
    }

    VulkanDescriptorSetLayout & VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout &&other) noexcept {
        if (this != &other) {
            DescriptorSetLayout::operator=(std::move(other));

            if (descriptorSetLayout != VK_NULL_HANDLE) {
                auto device = Renderer::getVulkanBackend()->getVkDevice();
                device.destroyDescriptorSetLayout(descriptorSetLayout);
            }

            descriptorSetLayout = other.descriptorSetLayout;
            other.descriptorSetLayout = VK_NULL_HANDLE;
        }
        return *this;
    }

    bool VulkanDescriptorSetLayout::isReady() const {
        return descriptorSetLayout != VK_NULL_HANDLE;
    }

    vk::DescriptorSetLayout VulkanDescriptorSetLayout::getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }
}
