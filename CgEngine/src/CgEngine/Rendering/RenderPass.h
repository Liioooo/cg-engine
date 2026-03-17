#pragma once

#include "CgEngineSharedUtils/Enums.h"
#include "DescriptorSetLayout.h"
#include "VertexBuffer.h"

namespace CgEngine {

    struct RenderPassSpecification {
        bool clearColorAttachments = true;
        bool clearDepthStencilAttachment = true;
        std::vector<AttachmentType> colorAttachments;
        bool hasDepthStencilAttachment = false;
        DepthStencilAttachmentFormat depthStencilAttachmentFormat;
        glm::vec4 clearColor;
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
