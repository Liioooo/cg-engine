#pragma once

#include "Rendering/ImmutableShaderStorageBuffer.h"

namespace CgEngine {

    class OpenGLImmutableShaderStorageBuffer : public ImmutableShaderStorageBuffer {
    public:
        OpenGLImmutableShaderStorageBuffer() = default;
        OpenGLImmutableShaderStorageBuffer(size_t size, const void* data);

        ~OpenGLImmutableShaderStorageBuffer() override;

        OpenGLImmutableShaderStorageBuffer(OpenGLImmutableShaderStorageBuffer&& other) noexcept;
        OpenGLImmutableShaderStorageBuffer& operator=(OpenGLImmutableShaderStorageBuffer&& other) noexcept;

        OpenGLImmutableShaderStorageBuffer(OpenGLImmutableShaderStorageBuffer& other) = delete;
        OpenGLImmutableShaderStorageBuffer& operator=(OpenGLImmutableShaderStorageBuffer& other) = delete;

        bool isReady() const override;
        size_t getSize() const override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t bufferHandle = ~0;
        size_t size;
    };

}
