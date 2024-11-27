#include "VertexArrayObject.h"
#include "Shader.h"
#include "glad/glad.h"
#include "Asserts.h"

namespace CgEngine {
    VertexArrayObject::VertexArrayObject(bool initVao) {
        if (initVao) {
            glGenVertexArrays(1, &vao);
        }
    }

    VertexArrayObject::~VertexArrayObject() {
        if (vao != ~0) {
            glDeleteVertexArrays(1, &vao);
        }

        if (!usingExistingIndexBuffer && ebo != ~0) {
            glDeleteBuffers(1, &ebo);
        }

        for (auto* item: vertexBuffers) {
            delete item;
        }
    }

    VertexArrayObject::VertexArrayObject(VertexArrayObject&& other) noexcept {
        vao = other.vao;
        ebo = other.ebo;
        usingExistingIndexBuffer = other.usingExistingIndexBuffer;
        vertexBufferIndex = other.vertexBufferIndex;
        indexCount = other.indexCount;
        vertexBuffers = std::move(other.vertexBuffers);

        other.vao = ~0;
        other.ebo = ~0;
        other.indexCount = 0;
    }

    VertexArrayObject& VertexArrayObject::operator=(VertexArrayObject&& other) noexcept {
        if (this != &other) {
            if (vao != ~0) {
                glDeleteVertexArrays(1, &vao);
            }
            if (!usingExistingIndexBuffer && ebo != ~0) {
                glDeleteBuffers(1, &ebo);
            }

            vao = other.vao;
            ebo = other.ebo;
            usingExistingIndexBuffer = other.usingExistingIndexBuffer;
            vertexBufferIndex = other.vertexBufferIndex;
            indexCount = other.indexCount;
            vertexBuffers = std::move(other.vertexBuffers);

            other.vao = ~0;
            other.ebo = ~0;
            other.indexCount = 0;
        }
        return *this;
    }

    void VertexArrayObject::bind() const {
        CG_ASSERT(isReady(), "VertexArrayObject is not ready!")
        glBindVertexArray(vao);
    }

    void VertexArrayObject::unbind() const {
        glBindVertexArray(0);
    }

    bool VertexArrayObject::isReady() const {
        return vao != ~0;
    }

    void VertexArrayObject::addVertexBuffer(VertexBuffer* buffer) {
        bind();
        buffer->bind();
        vertexBuffers.push_back(buffer);

        for (const auto &element: buffer->getLayout()) {
            switch (element.dataType) {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4: {
                    glEnableVertexAttribArray(vertexBufferIndex);
                    glVertexAttribPointer(
                            vertexBufferIndex,
                            element.getComponentCount(),
                            ShaderUtils::shaderDataTypeToOpenGLBaseType(element.dataType),
                            element.normalized ? GL_TRUE : GL_FALSE,
                            buffer->getStride(),
                            (const void*)(element.offset));
                    vertexBufferIndex++;
                    break;
                }
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                case ShaderDataType::Bool: {
                    glEnableVertexAttribArray(vertexBufferIndex);
                    glVertexAttribIPointer(
                            vertexBufferIndex,
                            element.getComponentCount(),
                            ShaderUtils::shaderDataTypeToOpenGLBaseType(element.dataType),
                            buffer->getStride(),
                            (const void*)(element.offset));
                    vertexBufferIndex++;
                    break;
                }
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4: {
                    uint8_t count = element.getComponentCount();
                    for (uint8_t i = 0; i < count; i++) {
                        glEnableVertexAttribArray(vertexBufferIndex);
                        glVertexAttribPointer(vertexBufferIndex,
                                              count,
                                              ShaderUtils::shaderDataTypeToOpenGLBaseType(element.dataType),
                                              element.normalized ? GL_TRUE : GL_FALSE,
                                              buffer->getStride(),
                                              (const void*)(element.offset + sizeof(float) * count * i));
                        glVertexAttribDivisor(vertexBufferIndex, 1);
                        vertexBufferIndex++;
                    }
                }
            }
        }

    }

    void VertexArrayObject::setIndexBuffer(const uint32_t* buffer, size_t count) {
        bind();
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), buffer, GL_STATIC_DRAW);
        indexCount = count;
    }

    void VertexArrayObject::useExistingIndexBuffer(uint32_t rendererId, size_t count) {
        bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererId);
        ebo = rendererId;
        indexCount = count;
        usingExistingIndexBuffer = true;
    }

    std::vector<VertexBuffer*>& VertexArrayObject::getVertexBuffers() {
        return vertexBuffers;
    }

    size_t VertexArrayObject::getIndexCount() const {
        return indexCount;
    }

    uint32_t VertexArrayObject::getRendererId() const {
        CG_ASSERT(isReady(), "VertexArrayObject is not ready!")
        return vao;
    }

    uint32_t VertexArrayObject::getIndexBufferRendererId() const {
        return ebo;
    }
}
