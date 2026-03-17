#pragma once

#include "Rendering/DynamicGraphicsPipeline.h"

namespace CgEngine {

    class OpenGLDynamicGraphicsPipeline : public DynamicGraphicsPipeline {
    public:
        OpenGLDynamicGraphicsPipeline() = default;
        OpenGLDynamicGraphicsPipeline(const DynamicGraphicsPipelineSpecification& spec);

        ~OpenGLDynamicGraphicsPipeline() override;

        OpenGLDynamicGraphicsPipeline(OpenGLDynamicGraphicsPipeline&& other) noexcept;
        OpenGLDynamicGraphicsPipeline& operator= (OpenGLDynamicGraphicsPipeline&& other) noexcept;

        OpenGLDynamicGraphicsPipeline(OpenGLDynamicGraphicsPipeline& other) = delete;
        OpenGLDynamicGraphicsPipeline& operator=(OpenGLDynamicGraphicsPipeline& other) = delete;

        bool isReady() const override;

        const DynamicGraphicsPipelineSpecification& getSpecification() const;
        uint32_t getOpenGLShaderHandle() const;
        unsigned int getDrawMode() const;

    private:
        DynamicGraphicsPipelineSpecification specification;
        uint32_t shaderHandle = ~0;
        bool ready = false;

    };
}
