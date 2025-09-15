#pragma once

#include "Rendering/VertexArrayObject.h"
#include "OpenGLIndexBuffer.h"
#include "OpenGLVertexBuffer.h"

namespace CgEngine {

    class OpenGLVertexArrayObject : public VertexArrayObject {
    public:
        OpenGLVertexArrayObject();

        ~OpenGLVertexArrayObject() override;

        OpenGLVertexArrayObject(OpenGLVertexArrayObject&& other) noexcept;
        OpenGLVertexArrayObject& operator=(OpenGLVertexArrayObject&& other) noexcept;

        OpenGLVertexArrayObject(OpenGLVertexArrayObject& other) = delete;
        OpenGLVertexArrayObject& operator=(OpenGLVertexArrayObject& other) = delete;

        void addVertexBuffer(VertexBuffer* buffer) override;
        void setIndexBuffer(const IndexBuffer* buffer) override;
        void useExistingIndexBuffer(const IndexBuffer* buffer) override;
        VertexBuffer* getVertexBuffer(uint32_t index) override;
        const IndexBuffer* getIndexBuffer() const override;
        const std::vector<VertexBufferLayout> getLayout() const override;

        void bind() const;
        uint32_t getOpenGLHandle() const;

    private:
        uint32_t vao = ~0;
        uint32_t vertexBufferIndex = 0;
        std::vector<OpenGLVertexBuffer*> vertexBuffers{};
        const OpenGLIndexBuffer* indexBuffer = nullptr;
        bool usingExistingIndexBuffer = false;
    };
}
