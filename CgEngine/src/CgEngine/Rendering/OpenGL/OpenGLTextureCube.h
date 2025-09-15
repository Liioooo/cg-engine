#pragma once

#include "Rendering/TextureCube.h"

namespace CgEngine {

    class OpenGLTextureCube : public TextureCube {
    public:
        OpenGLTextureCube() = default;
        OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);
        OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);

        ~OpenGLTextureCube() override;

        OpenGLTextureCube(OpenGLTextureCube&& other) noexcept;
        OpenGLTextureCube& operator=(OpenGLTextureCube&& other) noexcept;

        OpenGLTextureCube(OpenGLTextureCube& other) = delete;
        OpenGLTextureCube& operator=(OpenGLTextureCube& other) = delete;

        bool isReady() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;
        TextureFormat getFormat() const override;

        void generateMipMaps() override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t textureHandle = ~0;
        uint32_t width;
        uint32_t height;
        TextureFormat format;

    };
}
