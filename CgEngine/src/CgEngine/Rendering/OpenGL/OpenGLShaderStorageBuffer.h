#pragma once

#include "Rendering/ShaderStorageBuffer.h"

namespace CgEngine {

    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer {
    public:
        OpenGLShaderStorageBuffer() = default;
        explicit OpenGLShaderStorageBuffer(size_t size);
        OpenGLShaderStorageBuffer(size_t size, const void* data);

        ~OpenGLShaderStorageBuffer() override;

        OpenGLShaderStorageBuffer(OpenGLShaderStorageBuffer&& other) noexcept;
        OpenGLShaderStorageBuffer& operator=(OpenGLShaderStorageBuffer&& other) noexcept;

        OpenGLShaderStorageBuffer(OpenGLShaderStorageBuffer& other) = delete;
        OpenGLShaderStorageBuffer& operator=(OpenGLShaderStorageBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        void setData(const void* data, size_t size) override;
        void setSubData(size_t offset, const void* data, size_t size);

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t bufferHandle = ~0;
        size_t size;
    };

}
