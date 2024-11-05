#pragma once

#include "Shader.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {

    template<typename D>
    class UniformBuffer {
    public:
        UniformBuffer(const std::string& blockName, uint32_t binding, const Shader& shaderForInit) {
            init(blockName, binding, shaderForInit.getProgramId());
        }

        UniformBuffer(const std::string& blockName, uint32_t binding, const ComputeShader& shaderForInit) {
            init(blockName, binding, shaderForInit.getProgramId());
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

        void init(const std::string& blockName, uint32_t binding, uint32_t shaderForInitRendererId) {
            uint32_t shaderBlockIndex = glGetUniformBlockIndex(shaderForInitRendererId, blockName.c_str());
            glGetActiveUniformBlockiv(shaderForInitRendererId, shaderBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);

            glGenBuffers(1, &bufferId);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
            glNamedBufferData(bufferId, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        }
    };

}
