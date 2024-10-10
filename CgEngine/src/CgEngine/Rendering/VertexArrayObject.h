#pragma once

#include "VertexBuffer.h"

namespace CgEngine {

    class VertexArrayObject {
    public:
        VertexArrayObject();
        ~VertexArrayObject();

        void bind() const;
        void unbind() const;

        void addVertexBuffer(VertexBuffer* buffer);
        void setIndexBuffer(const uint32_t* buffer, size_t count);
        void useExistingIndexBuffer(uint32_t rendererId, size_t count);

        std::vector<VertexBuffer*>& getVertexBuffers();
        size_t getIndexCount() const;
        uint32_t getRendererId() const;
        uint32_t getIndexBufferRendererId() const;

    private:
        uint32_t vao{};
        uint32_t ebo{};
        size_t indexCount;
        uint32_t vertexBufferIndex = 0;
        std::vector<VertexBuffer*> vertexBuffers{};

        bool usingExistingIndexBuffer = false;
    };

}
