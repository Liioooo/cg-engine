#include <Asserts.h>
#include <glad/glad.h>
#include "OpenGLVertexArrayObject.h"
#include "OpenGLHelpers.h"

namespace CgEngine {

    OpenGLVertexArrayObject::OpenGLVertexArrayObject() : VertexArrayObject() {
        glGenVertexArrays(1, &vao);
    }

    OpenGLVertexArrayObject::~OpenGLVertexArrayObject() {
        if (vao != ~0) {
            glDeleteVertexArrays(1, &vao);
            vao = ~0;
        }

        if (!usingExistingIndexBuffer) {
            delete indexBuffer;
        }

        for (auto* item: vertexBuffers) {
            delete item;
        }
    }

    OpenGLVertexArrayObject::OpenGLVertexArrayObject(OpenGLVertexArrayObject&& other) noexcept : VertexArrayObject(std::move(other)) {
        vao = other.vao;
        usingExistingIndexBuffer = other.usingExistingIndexBuffer;
        vertexBufferIndex = other.vertexBufferIndex;
        vertexBuffers = std::move(other.vertexBuffers);
        indexBuffer = other.indexBuffer;
        other.indexBuffer = nullptr;
        other.vao = ~0;
        other.vertexBufferIndex = 0;
    }

    OpenGLVertexArrayObject& OpenGLVertexArrayObject::operator=(OpenGLVertexArrayObject&& other) noexcept {
        if (this != &other) {
            VertexArrayObject::operator=(std::move(other));

            if (vao != ~0) {
                glDeleteVertexArrays(1, &vao);
            }
            if (!usingExistingIndexBuffer) {
                delete indexBuffer;
            }

            vao = other.vao;
            usingExistingIndexBuffer = other.usingExistingIndexBuffer;
            vertexBufferIndex = other.vertexBufferIndex;
            vertexBuffers = std::move(other.vertexBuffers);
            indexBuffer = other.indexBuffer;
            other.indexBuffer = nullptr;
            other.vao = ~0;
            other.vertexBufferIndex = 0;
        }
        return *this;
    }

    void OpenGLVertexArrayObject::addVertexBuffer(VertexBuffer* buffer) {
        OpenGLVertexBuffer* openGlVertexBuffer = static_cast<OpenGLVertexBuffer*>(buffer);
        bind();
        glBindBuffer(GL_ARRAY_BUFFER, openGlVertexBuffer->getOpenGLHandle());
        vertexBuffers.push_back(openGlVertexBuffer);

        for (const auto &element : openGlVertexBuffer->getLayout().bufferElements) {
            switch (element.dataType) {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4: {
                    glEnableVertexAttribArray(vertexBufferIndex);
                    glVertexAttribPointer(
                            vertexBufferIndex,
                            element.getComponentCount(),
                            OpenGLHelpers::shaderDataTypeToOpenGLBaseType(element.dataType),
                            element.normalized ? GL_TRUE : GL_FALSE,
                            buffer->getLayout().stride,
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
                            OpenGLHelpers::shaderDataTypeToOpenGLBaseType(element.dataType),
                            buffer->getLayout().stride,
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
                                              OpenGLHelpers::shaderDataTypeToOpenGLBaseType(element.dataType),
                                              element.normalized ? GL_TRUE : GL_FALSE,
                                              buffer->getLayout().stride,
                                              (const void*)(element.offset + sizeof(float) * count * i));
                        glVertexAttribDivisor(vertexBufferIndex, 1);
                        vertexBufferIndex++;
                    }
                }
            }
        }
    }

    void OpenGLVertexArrayObject::setIndexBuffer(const IndexBuffer* buffer) {
        indexBuffer = static_cast<const OpenGLIndexBuffer*>(buffer);
        usingExistingIndexBuffer = false;

        bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->getOpenGLHandle());
    }

    void OpenGLVertexArrayObject::useExistingIndexBuffer(const IndexBuffer* buffer) {
        indexBuffer = static_cast<const OpenGLIndexBuffer*>(buffer);
        usingExistingIndexBuffer = true;

        bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->getOpenGLHandle());
    }

    VertexBuffer* OpenGLVertexArrayObject::getVertexBuffer(uint32_t index) {
        CG_ASSERT(index < vertexBuffers.size(), "Vertex buffer index out of range");
        return vertexBuffers[index];
    }

    const IndexBuffer* OpenGLVertexArrayObject::getIndexBuffer() const {
        return indexBuffer;
    }

    const std::vector<VertexBufferLayout> OpenGLVertexArrayObject::getLayout() const {
        std::vector<VertexBufferLayout> layouts;
        layouts.reserve(vertexBuffers.size());
        for (const auto& buffer: vertexBuffers) {
            layouts.push_back(buffer->getLayout());
        }
        return layouts;
    }

    void OpenGLVertexArrayObject::bind() const {
        CG_ASSERT(vao != ~0, "VAO not initialized");
        glBindVertexArray(vao);
    }

    uint32_t OpenGLVertexArrayObject::getOpenGLHandle() const {
        CG_ASSERT(vao != ~0, "VAO not initialized");
        return vao;
    }
}
