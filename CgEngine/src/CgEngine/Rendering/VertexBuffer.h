#pragma once

#include "Enums.h"

namespace CgEngine {

    struct VertexBufferElement {
        ShaderDataType dataType;
        int size;
        size_t offset{};
        bool normalized;

        VertexBufferElement(ShaderDataType dataType, bool normalized);

        bool operator ==(const VertexBufferElement& other) const {
            return dataType == other.dataType && normalized == other.normalized;
        }

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

    struct VertexBufferLayout {
        VertexBufferLayout() = default;
        explicit VertexBufferLayout(std::vector<VertexBufferElement> elements) : bufferElements(std::move(elements)) {
            size_t offset = 0;
            stride = 0;
            for (auto& e : bufferElements) {
                e.offset = offset;
                offset += e.size;
                stride += e.size;
            }
        }

        std::vector<VertexBufferElement> bufferElements;
        int stride = 0;
    };

    class VertexBuffer {
    public:
        VertexBuffer() = default;

        virtual ~VertexBuffer() = default;

        VertexBuffer(VertexBuffer&& other) noexcept = default;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept = default;

        VertexBuffer(VertexBuffer& other) = delete;
        VertexBuffer& operator=(VertexBuffer& other) = delete;

        virtual void setData(const void* data, size_t size) = 0;
        virtual void setLayout(VertexBufferLayout layout) = 0;
        virtual void setLayout(std::vector<VertexBufferElement> elements) = 0;

        virtual const VertexBufferLayout& getLayout() const = 0;
    };

}
