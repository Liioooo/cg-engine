#pragma once

namespace CgEngine {

    struct RenderPassSpecification {
        bool clearColorAttachments = true;
        bool clearDepthStencilAttachment = true;
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
