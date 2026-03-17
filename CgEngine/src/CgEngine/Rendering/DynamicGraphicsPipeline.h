#pragma once

#include "CgEngineSharedUtils/Enums.h"
#include "DescriptorSetLayout.h"
#include "VertexBuffer.h"
#include "RenderPass.h"

namespace CgEngine {

    struct DynamicGraphicsPipelineRenderingInfo {
        std::vector<AttachmentType> colorAttachments;
        bool hasDepthStencilAttachment = false;
        DepthStencilAttachmentFormat depthAttachmentFormat;
    };

    struct DynamicGraphicsPipelineSpecification {
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        std::vector<VertexBufferLayout> vertexInputLayout;
        std::vector<const DescriptorSetLayout*> descriptorSetLayouts;
        std::string engineShaderName;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        int tesselationPatchSize = ~0;
        DrawMode drawMode = DrawMode::Triangles;
        DynamicGraphicsPipelineRenderingInfo renderingInfo;
    };

    class DynamicGraphicsPipeline {
    public:
        DynamicGraphicsPipeline() = default;

        virtual ~DynamicGraphicsPipeline() = default;

        DynamicGraphicsPipeline(DynamicGraphicsPipeline&& other) noexcept = default;
        DynamicGraphicsPipeline& operator= (DynamicGraphicsPipeline&& other) noexcept = default;

        DynamicGraphicsPipeline(DynamicGraphicsPipeline& other) = delete;
        DynamicGraphicsPipeline& operator=(DynamicGraphicsPipeline& other) = delete;

        virtual bool isReady() const = 0;
    };
}
