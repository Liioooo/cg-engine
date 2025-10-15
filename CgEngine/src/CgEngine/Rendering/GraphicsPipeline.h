#pragma once

#include "Enums.h"
#include "DescriptorSetLayout.h"
#include "VertexBuffer.h"
#include "RenderPass.h"

namespace CgEngine {

    struct GraphicsPipelineSpecification {
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        std::vector<VertexBufferLayout> vertexInputLayout;
        const DescriptorSetLayout* descriptorSetLayout = nullptr;
        std::string engineShaderName;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        int tesselationPatchSize = ~0;
        DrawMode drawMode = DrawMode::Triangles;
        const RenderPass* renderPass = nullptr;
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
