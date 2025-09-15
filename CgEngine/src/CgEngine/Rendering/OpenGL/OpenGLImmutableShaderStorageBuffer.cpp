#pragma once

#include "OpenGLImmutableShaderStorageBuffer.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {

    OpenGLImmutableShaderStorageBuffer::OpenGLImmutableShaderStorageBuffer(size_t size, const void* data) {
        glCreateBuffers(1, &bufferHandle);
        glNamedBufferStorage(bufferHandle, size, data, 0);
    }

    OpenGLImmutableShaderStorageBuffer::~OpenGLImmutableShaderStorageBuffer() {
        if (bufferHandle != ~0) {
            glDeleteBuffers(1, &bufferHandle);
        }
    }

    OpenGLImmutableShaderStorageBuffer::OpenGLImmutableShaderStorageBuffer(OpenGLImmutableShaderStorageBuffer&& other) noexcept : ImmutableShaderStorageBuffer(std::move(other)) {
        bufferHandle = other.bufferHandle;
        size = other.size;

        other.bufferHandle = ~0;
    }

    OpenGLImmutableShaderStorageBuffer& OpenGLImmutableShaderStorageBuffer::operator=(OpenGLImmutableShaderStorageBuffer&& other) noexcept {
        if (this != &other) {
            ImmutableShaderStorageBuffer::operator=(std::move(other));

            if (bufferHandle != ~0) {
                glDeleteBuffers(1, &bufferHandle);
            }

            bufferHandle = other.bufferHandle;
            size = other.size;

            other.bufferHandle = ~0;
        }
        return *this;
    }

    bool OpenGLImmutableShaderStorageBuffer::isReady() const {
        return bufferHandle != ~0;
    }

    size_t OpenGLImmutableShaderStorageBuffer::getSize() const {
        CG_ASSERT(isReady(), "Shader Storage Buffer is not ready!")
        return size;
    }

    uint32_t OpenGLImmutableShaderStorageBuffer::getOpenGLHandle() const {
        CG_ASSERT(isReady(), "Shader Storage Buffer is not ready!")
        return bufferHandle;
    }

}
