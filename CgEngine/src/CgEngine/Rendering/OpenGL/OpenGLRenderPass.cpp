#include "OpenGLRenderPass.h"
#include "Asserts.h"

namespace CgEngine {

    OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec) : specification(spec) {
        ready = true;
    }

    OpenGLRenderPass::OpenGLRenderPass(OpenGLRenderPass&& other) noexcept : RenderPass(std::move(other)), ready(other.ready) {
        specification = std::move(other.specification);
        other.ready = false;
    }

    OpenGLRenderPass& OpenGLRenderPass::operator=(OpenGLRenderPass&& other) noexcept {
        if (this != &other) {
            RenderPass::operator=(std::move(other));

            specification = std::move(other.specification);
            other.specification = RenderPassSpecification();
            ready = other.ready;
            other.ready = false;
        }
        return *this;
    }

    bool OpenGLRenderPass::isReady() const {
        return ready;
    }

    const RenderPassSpecification& OpenGLRenderPass::getSpecification() const {
        CG_ASSERT(ready, "RenderPass is not ready!")
        return specification;
    }
}

