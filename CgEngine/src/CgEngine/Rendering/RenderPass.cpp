#include "RenderPass.h"

namespace CgEngine {
    RenderPass::RenderPass(RenderPassSpecification spec) : specification(std::move(spec)) {}

    RenderPass::~RenderPass() {
        if (!specification.usingExistingFramebuffer) {
            delete specification.framebuffer;
        }
    }

    RenderPass::RenderPass(RenderPass&& other) noexcept {
        specification = std::move(other.specification); // This just copies the pointer "framebuffer"
        other.specification.framebuffer = nullptr;
    }

    RenderPass& RenderPass::operator=(RenderPass&& other) noexcept {
        if (this != &other) {
            specification = std::move(other.specification); // This just copies the pointer "framebuffer"
            other.specification.framebuffer = nullptr;
        }

        return *this;
    }

    RenderPassSpecification& RenderPass::getSpecification() {
        return specification;
    }

    bool RenderPass::isReady() const {
        return specification.shader.isReady() && specification.framebuffer != nullptr;
    }
}
