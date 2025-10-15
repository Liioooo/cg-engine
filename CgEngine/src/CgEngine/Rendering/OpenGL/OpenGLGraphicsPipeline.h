#pragma once

#include "Rendering/GraphicsPipeline.h"

namespace CgEngine {

    class OpenGLGraphicsPipeline : public GraphicsPipeline {
    public:
        OpenGLGraphicsPipeline() = default;
        OpenGLGraphicsPipeline(const GraphicsPipelineSpecification& spec);

        ~OpenGLGraphicsPipeline() override;

        OpenGLGraphicsPipeline(OpenGLGraphicsPipeline&& other) noexcept;
        OpenGLGraphicsPipeline& operator= (OpenGLGraphicsPipeline&& other) noexcept;

        OpenGLGraphicsPipeline(OpenGLGraphicsPipeline& other) = delete;
        OpenGLGraphicsPipeline& operator=(OpenGLGraphicsPipeline& other) = delete;

        bool isReady() const override;

        const GraphicsPipelineSpecification& getSpecification() const;
        uint32_t getOpenGLShaderHandle() const;
        unsigned int getDrawMode() const;

    private:
        GraphicsPipelineSpecification specification;
        uint32_t shaderHandle = ~0;
        bool ready = false;

    };
}
