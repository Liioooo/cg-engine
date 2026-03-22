#pragma once

#include "Rendering/VertexBuffer.h"

namespace CgEngine {

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        explicit OpenGLVertexBuffer(size_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
        OpenGLVertexBuffer(const void* data, size_t size, VertexBufferUsage usage = VertexBufferUsage::Static);

        ~OpenGLVertexBuffer();

        OpenGLVertexBuffer(OpenGLVertexBuffer&& other) noexcept;
        OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other) noexcept;

        OpenGLVertexBuffer(OpenGLVertexBuffer& other) = delete;
        OpenGLVertexBuffer& operator=(OpenGLVertexBuffer& other) = delete;

        void setData(const void* data, size_t size) override;
        void setSubData(size_t offset, const void* data, size_t size) override;
        void setLayout(VertexBufferLayout layout) override;
        void setLayout(std::vector<VertexBufferElement> elements) override;

        const VertexBufferLayout& getLayout() const override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t vbo = ~0;
        VertexBufferLayout layout;
        VertexBufferUsage usage;

        static int getOpenGLUsage(VertexBufferUsage usage);
    };

}
