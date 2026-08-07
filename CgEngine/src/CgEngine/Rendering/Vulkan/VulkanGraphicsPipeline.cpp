#include "VulkanGraphicsPipeline.h"
#include "Logging.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification &spec) {
        VulkanHelpers::VulkanGraphicsShaderInfo shaderInfo;

        if (!spec.engineShaderName.empty()) {
            shaderInfo = VulkanHelpers::loadVulkanGraphicsShader(spec.engineShaderName, ShaderEnv::Engine);
        } else {
            shaderInfo = VulkanHelpers::loadVulkanGraphicsShader(spec.customShaders.vertex, spec.customShaders.fragment, spec.customShaders.geometry, spec.customShaders.tcs, spec.customShaders.tes, ShaderEnv::Custom);
        }

        const VulkanHelpers::VulkanPipelineVertexInputInfo vertexInputInfo = VulkanHelpers::getVulkanPipelineVertexInputInfo(spec.vertexInputLayout);

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.setTopology(VulkanHelpers::drawModeToVulkanPrimitiveTopology(spec.drawMode));
        inputAssembly.setPrimitiveRestartEnable(VK_FALSE);

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.setViewportCount(1);
        viewportState.setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.setDepthClampEnable(VK_FALSE);
        rasterizer.setRasterizerDiscardEnable(VK_FALSE);
        rasterizer.setPolygonMode(spec.wireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
        rasterizer.setLineWidth(1.0f);

        vk::CullModeFlags cullMode{};
        if (spec.backfaceCulling)
            cullMode |= vk::CullModeFlagBits::eBack;
        if (spec.frontfaceCulling)
            cullMode |= vk::CullModeFlagBits::eFront;

        rasterizer.setCullMode(cullMode);
        rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
        rasterizer.setDepthBiasEnable(VK_FALSE);
        rasterizer.setDepthBiasConstantFactor(0.0f);
        rasterizer.setDepthBiasClamp(0.0f);
        rasterizer.setDepthBiasSlopeFactor(0.0f);

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.setSampleShadingEnable(VK_FALSE);
        multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        multisampling.setMinSampleShading(1.0f);
        multisampling.setAlphaToCoverageEnable(VK_FALSE);
        multisampling.setAlphaToOneEnable(VK_FALSE);

        vk::PipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.setDepthTestEnable(spec.depthTest ? VK_TRUE : VK_FALSE);
        depthStencil.setDepthWriteEnable(spec.depthWrite ? VK_TRUE : VK_FALSE);
        depthStencil.setDepthCompareOp(VulkanHelpers::depthCompareOperatorToVulkan(spec.depthCompareOperator));
        depthStencil.setDepthBoundsTestEnable(VK_FALSE);
        depthStencil.setStencilTestEnable(VK_FALSE);

        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(spec.colorAttachments.size());

        for (auto& attachment : colorBlendAttachments) {
            attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            attachment.setBlendEnable(spec.useBlending);

            if (spec.useBlending) {
                attachment.setSrcColorBlendFactor(VulkanHelpers::blendingFunctionToVulkan(spec.srcBlendingFunction));
                attachment.setDstColorBlendFactor(VulkanHelpers::blendingFunctionToVulkan(spec.destBlendingFunction));
                attachment.setColorBlendOp(VulkanHelpers::blendingEquationToVulkan(spec.blendingEquation));

                attachment.setSrcAlphaBlendFactor(VulkanHelpers::blendingFunctionToVulkan(spec.srcBlendingFunction));
                attachment.setDstAlphaBlendFactor(VulkanHelpers::blendingFunctionToVulkan(spec.destBlendingFunction));
                attachment.setAlphaBlendOp(VulkanHelpers::blendingEquationToVulkan(spec.blendingEquation));
            }
        }

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.setAttachments(colorBlendAttachments);
        colorBlending.setLogicOpEnable(VK_FALSE);
        colorBlending.setBlendConstants(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});

        std::array dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.setDynamicStates(dynamicStates);

        vk::PipelineTessellationStateCreateInfo tessellation{};
        if (spec.drawMode == DrawMode::Patches) {
            tessellation.setPatchControlPoints(static_cast<uint32_t>(spec.tesselationPatchSize));
        }

        std::vector<vk::Format> colorFormats;
        colorFormats.reserve(spec.colorAttachments.size());

        for (auto attachment : spec.colorAttachments) {
            colorFormats.push_back(VulkanHelpers::attachmentTypeToVulkanColorFormat(attachment));
        }

        vk::PipelineRenderingCreateInfo renderingInfo{};
        renderingInfo.setColorAttachmentFormats(colorFormats);

        if (spec.hasDepthStencilAttachment) {
            auto depthFormat = VulkanHelpers::depthStencilAttachmentFormatToVulkanFormat(spec.depthAttachmentFormat);
            renderingInfo.setDepthAttachmentFormat(depthFormat);
            if (VulkanHelpers::hasFormatStencilComponent(depthFormat)) {
                renderingInfo.setStencilAttachmentFormat(depthFormat);
            }
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

        if (shaderInfo.fragmentModule != VK_NULL_HANDLE) {
            shaderStageFlags |= vk::ShaderStageFlagBits::eFragment;
        }
        if (shaderInfo.vertexModule != VK_NULL_HANDLE) {
            shaderStageFlags |= vk::ShaderStageFlagBits::eVertex;
        }
        if (shaderInfo.geometryModule != VK_NULL_HANDLE) {
            shaderStageFlags |= vk::ShaderStageFlagBits::eGeometry;
        }
        if (shaderInfo.tcsModule != VK_NULL_HANDLE) {
            shaderStageFlags |= vk::ShaderStageFlagBits::eTessellationControl;
        }
        if (shaderInfo.tesModule != VK_NULL_HANDLE) {
            shaderStageFlags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
        }

        vk::PushConstantRange pushConstantRange{};

        if (spec.usesPushConstants) {
            pushConstantRange.setOffset(0);
            pushConstantRange.setSize(spec.pushConstantsSize);
            pushConstantRange.setStageFlags(shaderStageFlags);

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

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.setStages(shaderInfo.shaderStages);
        pipelineInfo.setPVertexInputState(&vertexInputInfo.vertexInputInfo);
        pipelineInfo.setPInputAssemblyState(&inputAssembly);
        pipelineInfo.setPViewportState(&viewportState);
        pipelineInfo.setPRasterizationState(&rasterizer);
        pipelineInfo.setPMultisampleState(&multisampling);
        pipelineInfo.setPDepthStencilState(spec.hasDepthStencilAttachment ? &depthStencil : nullptr);
        pipelineInfo.setPColorBlendState(&colorBlending);
        pipelineInfo.setPDynamicState(&dynamicState);
        pipelineInfo.setRenderPass(VK_NULL_HANDLE);
        pipelineInfo.setSubpass(0);
        pipelineInfo.setBasePipelineHandle(VK_NULL_HANDLE);
        pipelineInfo.setLayout(pipelineLayout);
        pipelineInfo.setPNext(&renderingInfo);

        if (spec.drawMode == DrawMode::Patches) {
            pipelineInfo.setPTessellationState(&tessellation);
        }

        auto pipelineResult = device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);
        if (!pipelineResult.has_value()) {
            CG_LOGGING_ERROR("VulkanGraphicsPipeline: Failed to create pipeline!")
        }
        pipeline = pipelineResult.value;

        VulkanHelpers::destroyVulkanGraphicsShaderModules(shaderInfo);
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
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

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline &&other) noexcept : GraphicsPipeline(std::move(other)) {
        pipelineLayout = other.pipelineLayout;
        pipeline = other.pipeline;
        shaderStageFlags = other.shaderStageFlags;

        other.pipelineLayout = VK_NULL_HANDLE;
        other.pipeline = VK_NULL_HANDLE;
    }

    VulkanGraphicsPipeline & VulkanGraphicsPipeline::operator=(VulkanGraphicsPipeline &&other) noexcept {
        if (this != &other) {
            GraphicsPipeline::operator=(std::move(other));

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            if (pipeline != VK_NULL_HANDLE) {
                device.destroyPipeline(pipeline);
            }
            if (pipelineLayout != VK_NULL_HANDLE) {
                device.destroyPipelineLayout(pipelineLayout);
            }

            pipelineLayout = other.pipelineLayout;
            pipeline = other.pipeline;
            shaderStageFlags = other.shaderStageFlags;

            other.pipelineLayout = VK_NULL_HANDLE;
            other.pipeline = VK_NULL_HANDLE;
        }
        return *this;
    }

    bool VulkanGraphicsPipeline::isReady() const {
        return pipeline != VK_NULL_HANDLE;
    }

    vk::Pipeline VulkanGraphicsPipeline::getVulkanPipeline() const {
        return pipeline;
    }

    vk::PipelineLayout VulkanGraphicsPipeline::getVulkanPipelineLayout() const {
        return pipelineLayout;
    }

    vk::ShaderStageFlags VulkanGraphicsPipeline::getShaderStageFlags() const {
        return shaderStageFlags;
    }
}
