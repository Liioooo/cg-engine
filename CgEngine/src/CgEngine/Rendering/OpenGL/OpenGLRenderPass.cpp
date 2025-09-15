#include "OpenGLRenderPass.h"
#include "Asserts.h"
#include "OpenGLHelpers.h"
#include "glad/glad.h"

namespace CgEngine {

    OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec) : specification(spec) {
        shaderHandle = OpenGLHelpers::loadOpenGLGraphicsShader(spec.engineShaderName, ShaderEnv::Engine);
        if (shaderHandle == ~0) {
            CG_LOGGING_ERROR("Failed to load shader for RenderPass: {}", spec.engineShaderName)
            ready = false;
            return;
        }
        ready = true;
    }

    OpenGLRenderPass::~OpenGLRenderPass() {
        if (shaderHandle != ~0) {
            glDeleteProgram(shaderHandle);
        }
    }

    OpenGLRenderPass::OpenGLRenderPass(OpenGLRenderPass&& other) noexcept : RenderPass(std::move(other)), ready(other.ready) {
        specification = std::move(other.specification);
        shaderHandle = other.shaderHandle;
        other.shaderHandle = ~0;
        other.ready = false;
    }

    OpenGLRenderPass& OpenGLRenderPass::operator=(OpenGLRenderPass&& other) noexcept {
        if (this != &other) {
            RenderPass::operator=(std::move(other));

            if (shaderHandle != ~0) {
                glDeleteProgram(shaderHandle);
            }

            specification = std::move(other.specification);
            other.specification = RenderPassSpecification();
            shaderHandle = other.shaderHandle;
            other.shaderHandle = ~0;
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

    uint32_t OpenGLRenderPass::getOpenGLShaderHandle() const {
        CG_ASSERT(ready, "RenderPass is not ready!")
        return shaderHandle;
    }

    unsigned int OpenGLRenderPass::getDrawMode() const {
        return specification.tesselationPatchSize == ~0 ? GL_TRIANGLES : GL_PATCHES;
    }
}

