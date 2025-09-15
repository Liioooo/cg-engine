#pragma once

#include "Enums.h"

namespace CgEngine {

    class IndexBuffer {
    public:
        IndexBuffer() = default;
        IndexBuffer(uint32_t indexCount, IndexBufferDataType type = IndexBufferDataType::UInt32) : dataType(type), indexCount(indexCount) {};

        virtual ~IndexBuffer() = default;

        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;

        IndexBuffer(IndexBuffer& other) = delete;
        IndexBuffer& operator=(IndexBuffer& other) = delete;

        virtual void setData(const void* indices, uint32_t indexCount, IndexBufferDataType type = IndexBufferDataType::UInt32) = 0;
        virtual bool hasData() const = 0;
        IndexBufferDataType getDataType() const;
        uint32_t getIndexCount() const;

    protected:
        IndexBufferDataType dataType;
        uint32_t indexCount;

        size_t getSizeOfIndexBufferDataType(IndexBufferDataType type) const;
    };
}
