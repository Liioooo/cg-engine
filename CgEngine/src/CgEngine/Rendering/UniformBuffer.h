#pragma once

#include "Shader.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {

    template<typename D>
    class UniformBuffer {
    public:
        UniformBuffer() = default;

        UniformBuffer(const std::string& blockName, uint32_t binding, const Shader& shaderForInit) {
            init(blockName, binding, shaderForInit.getProgramId());
        }

        UniformBuffer(const std::string& blockName, uint32_t binding, const ComputeShader& shaderForInit) {
            init(blockName, binding, shaderForInit.getProgramId());
        }

        ~UniformBuffer() {
            if (bufferId != ~0) {
                glDeleteBuffers(1, &bufferId);
            }
        }

        UniformBuffer(UniformBuffer&& other) noexcept {
            bufferId = other.bufferId;
            bufferSize = other.bufferSize;

            other.bufferId = ~0;
        }

        UniformBuffer& operator=(UniformBuffer&& other) noexcept {
            if (this != &other) {
                if (bufferId != ~0) {
                    glDeleteBuffers(1, &bufferId);
                }

                bufferId = other.bufferId;
                bufferSize = other.bufferSize;

                other.bufferId = ~0;
            }
            return *this;
        }

        UniformBuffer(UniformBuffer& other) = delete;
        UniformBuffer& operator=(UniformBuffer& other) = delete;

        bool isReady() const {
            return bufferId != ~0;
        }

        void setData(const D& data) {
            glNamedBufferSubData(bufferId, 0, bufferSize, &data);
        }

    private:
        uint32_t bufferId = ~0;
        int bufferSize;

        void init(const std::string& blockName, uint32_t binding, uint32_t shaderForInitRendererId) {
            uint32_t shaderBlockIndex = glGetUniformBlockIndex(shaderForInitRendererId, blockName.c_str());
            glGetActiveUniformBlockiv(shaderForInitRendererId, shaderBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);

            CG_ASSERT(sizeof(D) == bufferSize, "Buffer size mismatch!")

            glGenBuffers(1, &bufferId);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
            glNamedBufferData(bufferId, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        }
    };

}
