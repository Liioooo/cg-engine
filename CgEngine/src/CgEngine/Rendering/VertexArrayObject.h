#pragma once

#include "VertexBuffer.h"

namespace CgEngine {

    class VertexArrayObject {
    public:
        explicit VertexArrayObject(bool initVao = true);
        ~VertexArrayObject();

        VertexArrayObject(VertexArrayObject&& other) noexcept;
        VertexArrayObject& operator=(VertexArrayObject&& other) noexcept;

        VertexArrayObject(VertexArrayObject& other) = delete;
        VertexArrayObject& operator=(VertexArrayObject& other) = delete;

        void bind() const;
        void unbind() const;

        bool isReady() const;

        void addVertexBuffer(VertexBuffer* buffer);
        void setIndexBuffer(const uint32_t* buffer, size_t count);
        void useExistingIndexBuffer(uint32_t rendererId, size_t count);

        std::vector<VertexBuffer*>& getVertexBuffers();
        size_t getIndexCount() const;
        uint32_t getRendererId() const;
        uint32_t getIndexBufferRendererId() const;

    private:
        uint32_t vao = ~0;
        uint32_t ebo = ~0;
        size_t indexCount;
        uint32_t vertexBufferIndex = 0;
        std::vector<VertexBuffer*> vertexBuffers{};

        bool usingExistingIndexBuffer = false;
    };

}
