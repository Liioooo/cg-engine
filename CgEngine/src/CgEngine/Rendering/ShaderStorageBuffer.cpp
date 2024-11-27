#include "ShaderStorageBuffer.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {
    ShaderStorageBuffer::ShaderStorageBuffer(bool initBuffer) {
        if (initBuffer) {
            glCreateBuffers(1, &bufferId);
        }
    }

    ShaderStorageBuffer::~ShaderStorageBuffer() {
        if (bufferId != ~0) {
            glDeleteBuffers(1, &bufferId);
        }
    }

    ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept {
        bufferId = other.bufferId;
        other.bufferId = ~0;
    }

    ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept {
        if (this != &other) {
            if (bufferId != ~0) {
                glDeleteBuffers(1, &bufferId);
            }

            bufferId = other.bufferId;
            other.bufferId = ~0;
        }
        return *this;
    }

    bool ShaderStorageBuffer::isReady() const {
        return bufferId != ~0;
    }

    void ShaderStorageBuffer::setData(const void* data, size_t size) {
        CG_ASSERT(isReady(), "ShaderStorageBuffer is not ready")
        glNamedBufferData(bufferId, size, data, GL_DYNAMIC_DRAW);
    }

    void ShaderStorageBuffer::setSubData(size_t offset, const void* data, size_t size) {
        CG_ASSERT(isReady(), "ShaderStorageBuffer is not ready")
        glNamedBufferSubData(bufferId, offset, size, data);
    }

    void ShaderStorageBuffer::bind(uint32_t binding) const {
        CG_ASSERT(isReady(), "ShaderStorageBuffer is not ready")
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, bufferId);
    }

}
