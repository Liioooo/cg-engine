#pragma once

#include "Rendering/ComputePipeline.h"

namespace CgEngine {

    class OpenGLComputePipeline : public ComputePipeline {
    public:
        OpenGLComputePipeline() = default;
        explicit OpenGLComputePipeline(const ComputePipelineSpecification& spec);

        ~OpenGLComputePipeline() override;

        OpenGLComputePipeline(OpenGLComputePipeline&& other) noexcept;
        OpenGLComputePipeline& operator=(OpenGLComputePipeline&& other) noexcept;

        OpenGLComputePipeline(OpenGLComputePipeline& other) = delete;
        OpenGLComputePipeline& operator=(OpenGLComputePipeline& other) = delete;

        bool isReady() const override;

        uint32_t getOpenGLShaderHandle() const;

    private:
        uint32_t shaderHandle = ~0;
    };

}
