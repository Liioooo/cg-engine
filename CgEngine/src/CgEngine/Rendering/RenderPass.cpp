#include "RenderPass.h"

namespace CgEngine {
    RenderPass::RenderPass(RenderPassSpecification spec) : specification(spec) {}

    RenderPass::~RenderPass() {
        if (!specification.usingExistingFramebuffer) {
            delete specification.framebuffer;
        }
        delete specification.shader;
    }

    const RenderPassSpecification& RenderPass::getSpecification() {
        return specification;
    }
}
