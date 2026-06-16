#pragma once

#include "CgEngineSharedUtils/Enums.h"
#include "DescriptorSetLayout.h"
#include "VertexBuffer.h"
#include "RenderPass.h"

namespace CgEngine {

    struct GraphicsPipelineCustomShaders {
        std::string vertex;
        std::string fragment;
        std::string geometry;
        std::string tcs;
        std::string tes;
    };

    struct GraphicsPipelineSpecification {
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        std::vector<VertexBufferLayout> vertexInputLayout;
        std::vector<const DescriptorSetLayout*> descriptorSetLayouts;
        std::string engineShaderName;
        GraphicsPipelineCustomShaders customShaders = {};
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        int tesselationPatchSize = ~0;
        DrawMode drawMode = DrawMode::Triangles;
        std::vector<AttachmentType> colorAttachments;
        bool hasDepthStencilAttachment = false;
        DepthStencilAttachmentFormat depthAttachmentFormat;
    };

    class GraphicsPipeline {
    public:
        GraphicsPipeline() = default;

        virtual ~GraphicsPipeline() = default;

        GraphicsPipeline(GraphicsPipeline&& other) noexcept = default;
        GraphicsPipeline& operator= (GraphicsPipeline&& other) noexcept = default;

        GraphicsPipeline(GraphicsPipeline& other) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline& other) = delete;

        virtual bool isReady() const = 0;
    };
}
