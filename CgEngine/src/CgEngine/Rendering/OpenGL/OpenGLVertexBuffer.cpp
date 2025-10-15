#include <Asserts.h>
#include "OpenGLVertexBuffer.h"
#include "glad/glad.h"

namespace CgEngine {

    OpenGLVertexBuffer::OpenGLVertexBuffer(VertexBufferUsage usage) : usage(usage) {}

    OpenGLVertexBuffer::OpenGLVertexBuffer(size_t size, VertexBufferUsage usage) : usage(usage) {
        glCreateBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, getOpenGLUsage(usage));
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(const void* data, size_t size, VertexBufferUsage usage) : usage(usage) {
        glCreateBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, getOpenGLUsage(usage));
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer() {
        if (vbo != ~0) {
            glDeleteBuffers(1, &vbo);
        }
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(OpenGLVertexBuffer&& other) noexcept : VertexBuffer(std::move(other)) {
        vbo = other.vbo;
        layout = other.layout;
        other.vbo = ~0;
    }

    OpenGLVertexBuffer& OpenGLVertexBuffer::operator=(OpenGLVertexBuffer&& other) noexcept {
        if (this != &other) {
            VertexBuffer::operator=(std::move(other));

            if (vbo != ~0) {
                glDeleteBuffers(1, &vbo);
            }
            vbo = other.vbo;
            layout = other.layout;
            other.vbo = ~0;
        }
        return *this;
    }

    void OpenGLVertexBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(vbo != ~0 || usage == VertexBufferUsage::Dynamic, "Static VertexBuffer cannot be updated")

        if (vbo == ~0) {
            glCreateBuffers(1, &vbo);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, getOpenGLUsage(usage));
    }

    void OpenGLVertexBuffer::setSubData(size_t offset, const void* data, size_t size) {
        CG_ASSERT(vbo != ~0 || usage == VertexBufferUsage::Dynamic, "Static VertexBuffer cannot be updated")

        if (vbo == ~0) {
            glCreateBuffers(1, &vbo);
        }
        glNamedBufferSubData(vbo, offset, size, data);
    }

    void OpenGLVertexBuffer::setLayout(VertexBufferLayout layout) {
        this->layout = layout;
    }

    void OpenGLVertexBuffer::setLayout(std::vector<VertexBufferElement> elements) {
        layout = VertexBufferLayout(elements);
    }

    const VertexBufferLayout& OpenGLVertexBuffer::getLayout() const {
        return layout;
    }

    uint32_t OpenGLVertexBuffer::getOpenGLHandle() const {
        CG_ASSERT(vbo != ~0, "OpenGLVertexBuffer has no OpenGL handle")
        return vbo;
    }

    int OpenGLVertexBuffer::getOpenGLUsage(CgEngine::VertexBufferUsage usage) {
        switch (usage) {
            case VertexBufferUsage::Static:
                return GL_STATIC_DRAW;
            case VertexBufferUsage::Dynamic:
                return GL_DYNAMIC_DRAW;
            default:
                return GL_STATIC_DRAW;
        }
    }

}
