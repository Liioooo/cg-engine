#include <Asserts.h>
#include "OpenGLGraphicsPipeline.h"
#include "OpenGLHelpers.h"
#include "glad/glad.h"

namespace CgEngine {

    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(const GraphicsPipelineSpecification& spec) : specification(spec) {
        if (!spec.engineShaderName.empty()) {
            shaderHandle = OpenGLHelpers::loadOpenGLGraphicsShader(spec.engineShaderName, ShaderEnv::Engine);
        } else {
            shaderHandle = OpenGLHelpers::loadOpenGLGraphicsShader(spec.customShaders.vertex, spec.customShaders.fragment, spec.customShaders.geometry, spec.customShaders.tcs, spec.customShaders.tes, ShaderEnv::Custom);
        }

        if (shaderHandle == ~0) {
            CG_LOGGING_ERROR("Failed to load shader for GraphicsPipeline: {}", spec.engineShaderName)
            ready = false;
            return;
        }
        ready = true;
    }

    OpenGLGraphicsPipeline::~OpenGLGraphicsPipeline() {
        if (shaderHandle != ~0) {
            glDeleteProgram(shaderHandle);
        }
    }

    OpenGLGraphicsPipeline::OpenGLGraphicsPipeline(OpenGLGraphicsPipeline&& other) noexcept {
        specification = std::move(other.specification);
        shaderHandle = other.shaderHandle;
        other.shaderHandle = ~0;
        other.ready = false;
    }

    OpenGLGraphicsPipeline& OpenGLGraphicsPipeline::operator=(OpenGLGraphicsPipeline&& other) noexcept {
        if (this != &other) {
            GraphicsPipeline::operator=(std::move(other));

            if (shaderHandle != ~0) {
                glDeleteProgram(shaderHandle);
            }

            specification = std::move(other.specification);
            other.specification = GraphicsPipelineSpecification();
            shaderHandle = other.shaderHandle;
            other.shaderHandle = ~0;
            ready = other.ready;
            other.ready = false;
        }
        return *this;
    }

    bool OpenGLGraphicsPipeline::isReady() const {
        return ready;
    }

    const GraphicsPipelineSpecification& OpenGLGraphicsPipeline::getSpecification() const {
        CG_ASSERT(ready, "GraphicsPipeline is not ready!")
        return specification;
    }

    uint32_t OpenGLGraphicsPipeline::getOpenGLShaderHandle() const {
        CG_ASSERT(ready, "GraphicsPipeline is not ready!")
        return shaderHandle;
    }

    unsigned int OpenGLGraphicsPipeline::getDrawMode() const {
        return OpenGLHelpers::getOpenGLDrawMode(specification.drawMode);
    }

}
