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
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const override;

        void bind() const;

    private:
        uint32_t shaderHandle = ~0;
    };

}
