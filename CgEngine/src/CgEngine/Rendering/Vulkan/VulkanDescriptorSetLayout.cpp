#include "VulkanDescriptorSetLayout.h"
#include "Asserts.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayoutSpecification &spec) {
        CG_ASSERT(validateBindingPoints({{spec.uboBindingPoints, "UBO"}, {spec.immutableSsboBindingPoints, "Immutable SSBO"}, {spec.ssboBindingPoints, "SSBO"}, {spec.vertexBufferSsboBindingPoints, "VertexBuffer SSBO"}, {spec.texture2DAndAttachmentBindingPoints, "Texture2D/Attachment"}, {spec.imageBindingPoints, "Image"}}), "DescriptorSetLayout: Binding points are overlapping!")

        bindingPointUsageMap = std::move(createBindingPointUsageMap(spec));

        std::vector<vk::DescriptorSetLayoutBinding> vulkanBindings;
        vulkanBindings.reserve(spec.uboBindingPoints.size() + spec.immutableSsboBindingPoints.size() + spec.ssboBindingPoints.size() + spec.vertexBufferSsboBindingPoints.size() + spec.texture2DAndAttachmentBindingPoints.size() + spec.imageBindingPoints.size());

        for (const auto& uboBindingPoint : spec.uboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = uboBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
            layoutBinding.descriptorCount = uboBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(uboBindingPoint.usage);
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& imSsboBindingPoint : spec.immutableSsboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = imSsboBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
            layoutBinding.descriptorCount = imSsboBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(imSsboBindingPoint.usage);;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& ssboBindingPoint : spec.ssboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = ssboBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageBufferDynamic;
            layoutBinding.descriptorCount = ssboBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(ssboBindingPoint.usage);;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& vertexSsboBindingPoint : spec.vertexBufferSsboBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = vertexSsboBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageBufferDynamic;
            layoutBinding.descriptorCount = vertexSsboBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(vertexSsboBindingPoint.usage);;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);

        }

        for (const auto& tex2DBindingPoint : spec.texture2DAndAttachmentBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = tex2DBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            layoutBinding.descriptorCount = tex2DBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(tex2DBindingPoint.usage);;
            layoutBinding.pImmutableSamplers = nullptr;
            vulkanBindings.push_back(layoutBinding);
        }

        for (const auto& imageBindingPoint : spec.imageBindingPoints) {
            vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = imageBindingPoint.bindingPoint;
            layoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
            layoutBinding.descriptorCount = imageBindingPoint.descriptorCount;
            layoutBinding.stageFlags = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(imageBindingPoint.usage);;
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
        bindingPointUsageMap = std::move(other.bindingPointUsageMap);
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
            bindingPointUsageMap = std::move(other.bindingPointUsageMap);
        }
        return *this;
    }

    DescriptorSetLayoutBindingUsage VulkanDescriptorSetLayout::getDescriptorSetLayoutBindingUsageForBindingPoint(uint32_t bindingPoint) const {
        return bindingPointUsageMap.at(bindingPoint);
    }

    bool VulkanDescriptorSetLayout::isReady() const {
        return descriptorSetLayout != VK_NULL_HANDLE;
    }

    vk::DescriptorSetLayout VulkanDescriptorSetLayout::getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }
}
