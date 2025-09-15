#include "OpenGLComputePipeline.h"
#include "glad/glad.h"
#include "OpenGLHelpers.h"
#include "Asserts.h"


namespace CgEngine {

    OpenGLComputePipeline::OpenGLComputePipeline(const ComputePipelineSpecification& spec) {
        shaderHandle = OpenGLHelpers::loadOpenGLComputeShader(spec.engineShaderName, ShaderEnv::Engine);
        if (shaderHandle == ~0) {
            CG_LOGGING_ERROR("Failed to load compute shader for ComputePipeline: {}", spec.engineShaderName)
            return;
        }

    }

    OpenGLComputePipeline::~OpenGLComputePipeline() {
        if (shaderHandle != ~0) {
            glDeleteProgram(shaderHandle);
        }
    }

    OpenGLComputePipeline::OpenGLComputePipeline(OpenGLComputePipeline&& other) noexcept : ComputePipeline(std::move(other)) {
        shaderHandle = other.shaderHandle;
        other.shaderHandle = ~0;
    }

    OpenGLComputePipeline& OpenGLComputePipeline::operator=(OpenGLComputePipeline&& other) noexcept {
        if (this != &other) {
            ComputePipeline::operator=(std::move(other));

            if (shaderHandle != ~0) {
                glDeleteProgram(shaderHandle);
            }

            shaderHandle = other.shaderHandle;
            other.shaderHandle = ~0;
        }
        return *this;
    }

    bool OpenGLComputePipeline::isReady() const {
        return shaderHandle != ~0;
    }

    void OpenGLComputePipeline::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
        CG_ASSERT(isReady(), "ComputePipeline is not ready!")
        glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    }

    void OpenGLComputePipeline::bind() const {
        CG_ASSERT(isReady(), "ComputePipeline is not ready!")
        glUseProgram(shaderHandle);
    }
}
