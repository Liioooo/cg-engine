#pragma once

#include "Rendering/RenderPass.h"

namespace CgEngine {

    class OpenGLRenderPass : public RenderPass {
    public:
        OpenGLRenderPass() = default;
        explicit OpenGLRenderPass(const RenderPassSpecification& spec);

        ~OpenGLRenderPass() override = default;

        OpenGLRenderPass(OpenGLRenderPass&& other) noexcept;
        OpenGLRenderPass& operator= (OpenGLRenderPass&& other) noexcept;

        OpenGLRenderPass(OpenGLRenderPass& other) = delete;
        OpenGLRenderPass& operator=(OpenGLRenderPass& other) = delete;

        bool isReady() const override;

        const RenderPassSpecification& getSpecification() const;

    private:
        RenderPassSpecification specification;
        bool ready = false;
    };
}
