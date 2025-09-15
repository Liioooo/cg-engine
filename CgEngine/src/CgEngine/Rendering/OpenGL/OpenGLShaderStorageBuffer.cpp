#pragma once

#include <Asserts.h>
#include "OpenGLShaderStorageBuffer.h"
#include "glad/glad.h"

namespace CgEngine {

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size) : size(size) {
        glCreateBuffers(1, &bufferHandle);
        glNamedBufferData(bufferHandle, size, nullptr, GL_DYNAMIC_DRAW);
    }

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(size_t size, const void* data) {
        glCreateBuffers(1, &bufferHandle);
        glNamedBufferData(bufferHandle, size, data, GL_DYNAMIC_DRAW);
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer() {
        if (bufferHandle != ~0) {
            glDeleteBuffers(1, &bufferHandle);
        }
    }

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(OpenGLShaderStorageBuffer&& other) noexcept : ShaderStorageBuffer(std::move(other)) {
        bufferHandle = other.bufferHandle;
        size = other.size;

        other.bufferHandle = ~0;
    }

    OpenGLShaderStorageBuffer& OpenGLShaderStorageBuffer::operator=(OpenGLShaderStorageBuffer&& other) noexcept {
        if (this != &other) {
            ShaderStorageBuffer::operator=(std::move(other));

            if (bufferHandle != ~0) {
                glDeleteBuffers(1, &bufferHandle);
            }

            bufferHandle = other.bufferHandle;
            size = other.size;

            other.bufferHandle = ~0;
        }
        return *this;
    }

    bool OpenGLShaderStorageBuffer::isReady() const {
        return bufferHandle != ~0;
    }

    size_t OpenGLShaderStorageBuffer::getSize() const {
        CG_ASSERT(isReady(), "OpenGLShaderStorageBuffer is not ready")
        return size;
    }

    void OpenGLShaderStorageBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(isReady(), "OpenGLShaderStorageBuffer is not ready")
        CG_ASSERT(size == this->size, "OpenGLShaderStorageBuffer::setData: Data size does not match the OpenGLShaderStorageBuffer size.")
        glNamedBufferData(bufferHandle, size, data, GL_DYNAMIC_DRAW);
    }

    void OpenGLShaderStorageBuffer::setSubData(size_t offset, const void* data, size_t size) {
        CG_ASSERT(isReady(), "OpenGLShaderStorageBuffer is not ready")
        CG_ASSERT(offset + size <= this->size, "ShaderStorageBuffer::setSubData: Data size and offset exceed the buffer size.")
        glNamedBufferSubData(bufferHandle, offset, size, data);
    }

    uint32_t OpenGLShaderStorageBuffer::getOpenGLHandle() const {
        CG_ASSERT(isReady(), "OpenGLShaderStorageBuffer::getOpenGLHandle: Buffer is not ready.")
        return bufferHandle;
    }

}
