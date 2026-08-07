#include "VulkanComputePipeline.h"

#include "Asserts.h"
#include "Logging.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanComputePipeline::VulkanComputePipeline(const ComputePipelineSpecification &spec) {
        VulkanHelpers::VulkanComputeShaderInfo shaderInfo;

        if (!spec.engineShaderName.empty()) {
            shaderInfo = VulkanHelpers::loadVulkanComputeShader(spec.engineShaderName, ShaderEnv::Engine);
        } else {
            shaderInfo = VulkanHelpers::loadVulkanCustomComputeShader(spec.customShader);
        }

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};

        std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts{};
        if (!spec.descriptorSetLayouts.empty()) {
            vkDescriptorSetLayouts.reserve(spec.descriptorSetLayouts.size());
            for (const auto& layout : spec.descriptorSetLayouts) {
                auto vkLayout = static_cast<const VulkanDescriptorSetLayout*>(layout)->getDescriptorSetLayout();
                vkDescriptorSetLayouts.push_back(vkLayout);
            }
            pipelineLayoutInfo.setSetLayouts(vkDescriptorSetLayouts);
        } else {
            pipelineLayoutInfo.setSetLayoutCount(0);
            pipelineLayoutInfo.setSetLayouts(nullptr);
        }

        vk::PushConstantRange pushConstantRange{};

        if (spec.usesPushConstants) {
            pushConstantRange.setOffset(0);
            pushConstantRange.setSize(spec.pushConstantsSize);
            pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eCompute);

            pipelineLayoutInfo.setPPushConstantRanges(&pushConstantRange);
            pipelineLayoutInfo.setPushConstantRangeCount(1);
        } else {
            pipelineLayoutInfo.setPushConstantRangeCount(0);
        }

        auto device = Renderer::getVulkanBackend()->getVkDevice();

        auto pipelineLayoutResult = device.createPipelineLayout(pipelineLayoutInfo);
        if (!pipelineLayoutResult.has_value()) {
            CG_LOGGING_ERROR("VulkanGraphicsPipeline: Failed to create pipeline layout!")
        }
        pipelineLayout = pipelineLayoutResult.value;

        vk::ComputePipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.setStage(shaderInfo.shaderStage);
        pipelineCreateInfo.setLayout(pipelineLayout);

        auto pipelineResult = device.createComputePipeline(VK_NULL_HANDLE, pipelineCreateInfo);
        if (!pipelineResult.has_value()) {
            CG_LOGGING_ERROR("VulkanGraphicsPipeline: Failed to create pipeline!")
        }
        pipeline = pipelineResult.value;

        VulkanHelpers::destroyVulkanComputeShaderModule(shaderInfo);
    }

    VulkanComputePipeline::~VulkanComputePipeline() {
        auto device = Renderer::getVulkanBackend()->getVkDevice();

        if (pipeline != VK_NULL_HANDLE) {
            device.destroyPipeline(pipeline);
            pipeline = VK_NULL_HANDLE;
        }

        if (pipelineLayout != VK_NULL_HANDLE) {
            device.destroyPipelineLayout(pipelineLayout);
            pipelineLayout = VK_NULL_HANDLE;
        }
    }

    VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline &&other) noexcept : ComputePipeline(std::move(other)) {
        pipelineLayout = other.pipelineLayout;
        pipeline = other.pipeline;

        other.pipelineLayout = VK_NULL_HANDLE;
        other.pipeline = VK_NULL_HANDLE;
    }

    VulkanComputePipeline & VulkanComputePipeline::operator=(VulkanComputePipeline &&other) noexcept {
        if (this != &other) {
            ComputePipeline::operator=(std::move(other));

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            if (pipeline != VK_NULL_HANDLE) {
                device.destroyPipeline(pipeline);
            }
            if (pipelineLayout != VK_NULL_HANDLE) {
                device.destroyPipelineLayout(pipelineLayout);
            }

            pipelineLayout = other.pipelineLayout;
            pipeline = other.pipeline;

            other.pipelineLayout = VK_NULL_HANDLE;
            other.pipeline = VK_NULL_HANDLE;
        }
        return *this;
    }

    bool VulkanComputePipeline::isReady() const {
        return pipeline != VK_NULL_HANDLE;
    }

    vk::Pipeline VulkanComputePipeline::getVulkanPipeline() const {
        CG_ASSERT(isReady(), "VulkanComputePipeline is not ready!")
        return pipeline;
    }

    vk::PipelineLayout VulkanComputePipeline::getVulkanPipelineLayout() const {
        CG_ASSERT(isReady(), "VulkanComputePipeline is not ready!")
        return pipelineLayout;
    }
}
