#pragma once

#include "Shader.h"

namespace CgEngine {

    enum class VertexBufferUsage {
        Static, Dynamic
    };

    static unsigned int openGLUsage(VertexBufferUsage usage);

    struct VertexBufferElement {
        ShaderDataType dataType;
        int size;
        size_t offset{};
        bool normalized;

        VertexBufferElement(ShaderDataType dataType, bool normalized) : dataType(dataType), normalized(normalized), size(ShaderUtils::getSizeForShaderDataType(dataType)) {};

        int getComponentCount() const {
            switch (dataType) {
                case ShaderDataType::Float:   return 1;
                case ShaderDataType::Float2:  return 2;
                case ShaderDataType::Float3:  return 3;
                case ShaderDataType::Float4:  return 4;
                case ShaderDataType::Mat3:    return 3;
                case ShaderDataType::Mat4:    return 4;
                case ShaderDataType::Int:     return 1;
                case ShaderDataType::Int2:    return 2;
                case ShaderDataType::Int3:    return 3;
                case ShaderDataType::Int4:    return 4;
                case ShaderDataType::Bool:    return 1;
            }

            return 0;
        }
    };

    class VertexBuffer {
    public:
        explicit VertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
        VertexBuffer(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
        ~VertexBuffer();

        void bind() const;
        void bindAsSSBO(uint32_t binding) const;
        void unbind() const;
        void setData(const void* data, uint32_t size, VertexBufferUsage usage);
        void setLayout(std::vector<VertexBufferElement> bufferElements);

        const std::vector<VertexBufferElement>& getLayout() const;
        int getStride() const;

    private:
        uint32_t vbo{};
        std::vector<VertexBufferElement> bufferElements;
        int stride = 0;
    };

}
