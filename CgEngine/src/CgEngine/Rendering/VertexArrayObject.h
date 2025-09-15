#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace CgEngine {

    class VertexArrayObject {
    public:
        VertexArrayObject() = default;

        virtual ~VertexArrayObject() = default;

        VertexArrayObject(VertexArrayObject&& other) noexcept = default;
        VertexArrayObject& operator=(VertexArrayObject&& other) noexcept = default;

        VertexArrayObject(VertexArrayObject& other) = delete;
        VertexArrayObject& operator=(VertexArrayObject& other) = delete;

        virtual void addVertexBuffer(VertexBuffer* buffer) = 0;
        virtual void setIndexBuffer(const IndexBuffer* buffer) = 0;
        virtual void useExistingIndexBuffer(const IndexBuffer* buffer) = 0;
        virtual VertexBuffer* getVertexBuffer(uint32_t index) = 0;
        virtual const IndexBuffer* getIndexBuffer() const = 0;
        virtual const std::vector<VertexBufferLayout> getLayout() const = 0;
    };

}
