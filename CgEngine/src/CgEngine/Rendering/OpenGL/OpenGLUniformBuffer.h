#pragma once

#include "Rendering/UniformBuffer.h"

namespace CgEngine {

    class OpenGLUniformBuffer : public UniformBuffer {
    public:
        OpenGLUniformBuffer() = default;
        explicit OpenGLUniformBuffer(size_t size);

        ~OpenGLUniformBuffer() override;

        OpenGLUniformBuffer(OpenGLUniformBuffer&& other) noexcept;
        OpenGLUniformBuffer& operator=(OpenGLUniformBuffer&& other) noexcept;

        OpenGLUniformBuffer(OpenGLUniformBuffer& other) = delete;
        OpenGLUniformBuffer& operator=(OpenGLUniformBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        void setData(const void* data, size_t size) override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t bufferHandle = ~0;
        size_t size;
    };

}
