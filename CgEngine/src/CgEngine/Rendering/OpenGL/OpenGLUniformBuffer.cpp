#include "OpenGLUniformBuffer.h"
#include "Asserts.h"
#include "glad/glad.h"

namespace CgEngine {
    OpenGLUniformBuffer::OpenGLUniformBuffer(size_t size) : size(size) {
        glCreateBuffers(1, &bufferHandle);
    }

    OpenGLUniformBuffer::OpenGLUniformBuffer(OpenGLUniformBuffer&& other) noexcept : UniformBuffer(std::move(other)) {
        bufferHandle = other.bufferHandle;
        size = other.size;

        other.bufferHandle = ~0;
    }

    OpenGLUniformBuffer& OpenGLUniformBuffer::operator=(OpenGLUniformBuffer&& other) noexcept {
        if (this != &other) {
            UniformBuffer::operator=(std::move(other));

            if (bufferHandle != ~0) {
                glDeleteBuffers(1, &bufferHandle);
            }

            bufferHandle = other.bufferHandle;
            size = other.size;

            other.bufferHandle = ~0;
        }
        return *this;
}

    OpenGLUniformBuffer::~OpenGLUniformBuffer() {
        if (bufferHandle != ~0) {
            glDeleteBuffers(1, &bufferHandle);
        }
    }

    bool OpenGLUniformBuffer::isReady() const {
        return bufferHandle != ~0;
    }

    size_t OpenGLUniformBuffer::getSize() const {
        CG_ASSERT(isReady(), "OpenGLUniformBuffer is not ready")
        return size;
    }

    void OpenGLUniformBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(isReady(), "OpenGLUniformBuffer is not ready")
        CG_ASSERT(size == this->size, "OpenGLUniformBuffer::setData: Data size does not match the UniformBuffer size.")
        glNamedBufferData(bufferHandle, size, data, GL_DYNAMIC_DRAW);
    }

    uint32_t OpenGLUniformBuffer::getOpenGLHandle() const {
        CG_ASSERT(isReady(), "OpenGLUniformBuffer is not ready")
        return bufferHandle;
    }
}
