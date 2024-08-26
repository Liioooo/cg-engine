#pragma once

#include "Shader.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {

    template<typename D>
    class UniformBuffer {
    public:
        UniformBuffer(const std::string& blockName, uint32_t binding, Shader& shaderForInit) {
            uint32_t shaderBlockIndex = glGetUniformBlockIndex(shaderForInit.getProgramId(), blockName.c_str());
            glGetActiveUniformBlockiv(shaderForInit.getProgramId(), shaderBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);

            glGenBuffers(1, &bufferId);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
            glNamedBufferData(bufferId, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        }

        ~UniformBuffer() {
            glDeleteBuffers(1, &bufferId);
        }

        void setData(const D& data) {
            CG_ASSERT(sizeof(data) == bufferSize, "Buffer size mismatch!")

            glNamedBufferSubData(bufferId, 0, bufferSize, &data);
        }

    private:
        uint32_t bufferId;
        int bufferSize;
    };

}
