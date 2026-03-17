#pragma once

#include "Rendering/VertexBuffer.h"

namespace CgEngine {

    struct UiDrawCommand {
        uint32_t circleIndexCount;
        uint8_t circleVertexCount;
        const VertexArrayObject* circleVAO;
        uint32_t rectIndexCount;
        uint8_t rectVertexCount;
        const VertexArrayObject* rectVAO;
        const DescriptorSet* descriptorSet;

        uint32_t textIndexCount;
        uint8_t textVertexCount;
        const VertexArrayObject* textVAO;
        const DescriptorSet* textDescriptorSet;
    };

    static inline const std::vector<VertexBufferElement> UI_CIRCLE_VERTEX_BUFFER_LAYOUT = {
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float, false),
            VertexBufferElement(ShaderDataType::Float, false),
            VertexBufferElement(ShaderDataType::Float, false)
    };

    static inline const std::vector<VertexBufferLayout> UI_CIRCLE_VERTEX_BUFFER_LAYOUTS = {
            VertexBufferLayout(UI_CIRCLE_VERTEX_BUFFER_LAYOUT)
    };

    static inline const std::vector<VertexBufferElement> UI_RECT_VERTEX_BUFFER_LAYOUT = {
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float2, false),
            VertexBufferElement(ShaderDataType::Float, false),
            VertexBufferElement(ShaderDataType::Float, false)
    };

    static inline const std::vector<VertexBufferLayout> UI_RECT_VERTEX_BUFFER_LAYOUTS = {
            VertexBufferLayout(UI_RECT_VERTEX_BUFFER_LAYOUT)
    };

    static inline const std::vector<VertexBufferElement> UI_TEXT_VERTEX_BUFFER_LAYOUT = {
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float4, false),
            VertexBufferElement(ShaderDataType::Float, false)
    };

    static inline const std::vector<VertexBufferLayout> UI_TEXT_VERTEX_BUFFER_LAYOUTS = {
            VertexBufferLayout(UI_TEXT_VERTEX_BUFFER_LAYOUT)
    };

    static inline const uint32_t MAX_UI_Z_LAYERS = 10;
    static inline const uint32_t MAX_UI_QUADS = 5000;
    static inline const uint32_t MAX_UI_INDICES = MAX_UI_QUADS * 6;
    static inline const uint32_t MAX_UI_VERTICES = MAX_UI_QUADS * 4 * 2;

    static inline const AttachmentType UI_CANVAS_ATTACHMENT_TYPE = AttachmentType::RGBA8;


}
