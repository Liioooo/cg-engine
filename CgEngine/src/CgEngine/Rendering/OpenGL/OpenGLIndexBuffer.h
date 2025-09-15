#pragma once

#include "Rendering/IndexBuffer.h"

namespace CgEngine {

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer() = default;
        OpenGLIndexBuffer(const void* indices, uint32_t indexCount, IndexBufferDataType type = IndexBufferDataType::UInt32);

        ~OpenGLIndexBuffer() override;

        OpenGLIndexBuffer(OpenGLIndexBuffer&& other) noexcept;
        OpenGLIndexBuffer& operator=(OpenGLIndexBuffer&& other) noexcept;

        OpenGLIndexBuffer(OpenGLIndexBuffer& other) = delete;
        OpenGLIndexBuffer& operator=(OpenGLIndexBuffer& other) = delete;

        void setData(const void* indices, uint32_t indexCount, IndexBufferDataType type) override;
        bool hasData() const override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t ebo = ~0;
    };

}
