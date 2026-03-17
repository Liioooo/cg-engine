#include <Asserts.h>
#include "OpenGLDynamicGraphicsPipeline.h"
#include "OpenGLHelpers.h"
#include "glad/glad.h"

namespace CgEngine {

    OpenGLDynamicGraphicsPipeline::OpenGLDynamicGraphicsPipeline(const DynamicGraphicsPipelineSpecification& spec) : specification(spec) {
        shaderHandle = OpenGLHelpers::loadOpenGLGraphicsShader(spec.engineShaderName, ShaderEnv::Engine);

        if (shaderHandle == ~0) {
            CG_LOGGING_ERROR("Failed to load shader for DynamicGraphicsPipeline: {}", spec.engineShaderName)
            ready = false;
            return;
        }
        ready = true;
    }

    OpenGLDynamicGraphicsPipeline::~OpenGLDynamicGraphicsPipeline() {
        if (shaderHandle != ~0) {
            glDeleteProgram(shaderHandle);
        }
    }

    OpenGLDynamicGraphicsPipeline::OpenGLDynamicGraphicsPipeline(OpenGLDynamicGraphicsPipeline&& other) noexcept {
        specification = std::move(other.specification);
        shaderHandle = other.shaderHandle;
        other.shaderHandle = ~0;
        other.ready = false;
    }

    OpenGLDynamicGraphicsPipeline& OpenGLDynamicGraphicsPipeline::operator=(OpenGLDynamicGraphicsPipeline&& other) noexcept {
        if (this != &other) {
            DynamicGraphicsPipeline::operator=(std::move(other));

            if (shaderHandle != ~0) {
                glDeleteProgram(shaderHandle);
            }

            specification = std::move(other.specification);
            other.specification = DynamicGraphicsPipelineSpecification();
            shaderHandle = other.shaderHandle;
            other.shaderHandle = ~0;
            ready = other.ready;
            other.ready = false;
        }
        return *this;
    }

    bool OpenGLDynamicGraphicsPipeline::isReady() const {
        return ready;
    }

    const DynamicGraphicsPipelineSpecification& OpenGLDynamicGraphicsPipeline::getSpecification() const {
        CG_ASSERT(ready, "GraphicsPipeline is not ready!")
        return specification;
    }

    uint32_t OpenGLDynamicGraphicsPipeline::getOpenGLShaderHandle() const {
        CG_ASSERT(ready, "GraphicsPipeline is not ready!")
        return shaderHandle;
    }

    unsigned int OpenGLDynamicGraphicsPipeline::getDrawMode() const {
        return OpenGLHelpers::getOpenGLDrawMode(specification.drawMode);
    }

}
