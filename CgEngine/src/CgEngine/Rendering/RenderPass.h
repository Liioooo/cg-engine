#pragma once

#include "Enums.h"
#include "DescriptorSetLayout.h"
#include "VertexBuffer.h"

namespace CgEngine {

    struct RenderPassSpecification {
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        bool clearDepthAttachment = true;
        bool clearColorAttachments = true;
        bool clearStencilBuffer = false;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        std::vector<AttachmentType> colorAttachments;
        bool queryColorAttachmentFormatFromSwapChain = false;
        bool hasDepthStencilAttachment = false;
        DepthAttachmentFormat depthAttachmentFormat;
        glm::vec4 clearColor;
        std::vector<VertexBufferLayout> vertexInputLayout;
        const DescriptorSetLayout* descriptorSetLayout = nullptr;
        std::string engineShaderName;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        int tesselationPatchSize = ~0;
    };

    class RenderPass {
    public:
        RenderPass() = default;

        virtual ~RenderPass() = default;

        RenderPass(RenderPass&& other) noexcept = default;
        RenderPass& operator= (RenderPass&& other) noexcept = default;

        RenderPass(RenderPass& other) = delete;
        RenderPass& operator=(RenderPass& other) = delete;

        virtual bool isReady() const = 0;
    };

}
