#include <glad/glad.h>
#include <Asserts.h>
#include "OpenGLIndexBuffer.h"

namespace CgEngine {

    OpenGLIndexBuffer::OpenGLIndexBuffer(const void* indices, uint32_t indexCount, CgEngine::IndexBufferDataType type) : IndexBuffer(indexCount, type) {
        glCreateBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * getSizeOfIndexBufferDataType(type), indices, GL_STATIC_DRAW);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer() {
        if (ebo != ~0) {
            glDeleteBuffers(1, &ebo);
        }
    }

    OpenGLIndexBuffer::OpenGLIndexBuffer(OpenGLIndexBuffer&& other) noexcept : IndexBuffer(std::move(other)) {
        ebo = other.ebo;
        other.ebo = ~0;
    }

    OpenGLIndexBuffer& OpenGLIndexBuffer::operator=(OpenGLIndexBuffer&& other) noexcept {
        if (this != &other) {
            IndexBuffer::operator=(std::move(other));

            if (ebo != ~0) {
                glDeleteBuffers(1, &ebo);
            }

            ebo = other.ebo;
            other.ebo = ~0;
        }
        return *this;
    }

    void OpenGLIndexBuffer::setData(const void* indices, uint32_t indexCount, IndexBufferDataType type) {
        CG_ASSERT(!hasData(), "OpenGLIndexBuffer::setData: Buffer already has data. You cannot change data of an existing IndexBuffer.")

        glCreateBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * getSizeOfIndexBufferDataType(type), indices, GL_STATIC_DRAW);
    }

    bool OpenGLIndexBuffer::hasData() const {
        return ebo != ~0;
    }

    uint32_t OpenGLIndexBuffer::getOpenGLHandle() const {
        CG_ASSERT(hasData(), "OpenGLIndexBuffer::getOpenGLHandle: Buffer has no data.")
        return ebo;
    }
}
