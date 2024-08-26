#include "VertexBuffer.h"
#include "glad/glad.h"

namespace CgEngine {
    static unsigned int openGLUsage(VertexBufferUsage usage) {
        switch (usage) {
            case VertexBufferUsage::Static:    return GL_STATIC_DRAW;
            case VertexBufferUsage::Dynamic:   return GL_DYNAMIC_DRAW;
        }
        return 0;
    }

    VertexBuffer::VertexBuffer(uint32_t size, VertexBufferUsage usage) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, openGLUsage(usage));
    }

    VertexBuffer::VertexBuffer(void* data, uint32_t size, VertexBufferUsage usage) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, openGLUsage(usage));
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &vbo);
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    void VertexBuffer::bindAsSSBO(uint32_t binding) const {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, vbo);
    }

    void VertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::setData(const void *data, uint32_t size, VertexBufferUsage usage) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, data, openGLUsage(usage));
    }

    void VertexBuffer::setLayout(std::vector<VertexBufferElement> elements) {
        bufferElements = elements;

        size_t offset = 0;
        stride = 0;
        for (auto& e : bufferElements) {
            e.offset = offset;
            offset += e.size;
            stride += e.size;
        }
    }

    const std::vector<VertexBufferElement> &VertexBuffer::getLayout() const {
        return bufferElements;
    }

    int VertexBuffer::getStride() const {
        return stride;
    }
}
