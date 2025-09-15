#include "IndexBuffer.h"

namespace CgEngine {

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept {
        dataType = other.dataType;
        indexCount = other.indexCount;
        other.indexCount = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
        if (this != &other) {
            dataType = other.dataType;
            indexCount = other.indexCount;
            other.indexCount = 0;
        }
        return *this;
    }

    IndexBufferDataType IndexBuffer::getDataType() const {
        return dataType;
    }

    uint32_t IndexBuffer::getIndexCount() const {
        return indexCount;
    }

    size_t IndexBuffer::getSizeOfIndexBufferDataType(CgEngine::IndexBufferDataType type) const {
        switch (type) {
            case IndexBufferDataType::UInt8:
                return sizeof(uint8_t);
            case IndexBufferDataType::UInt16:
                return sizeof(uint16_t);
            case IndexBufferDataType::UInt32:
                return sizeof(uint32_t);
        }
        return sizeof(uint32_t);
    }
}
