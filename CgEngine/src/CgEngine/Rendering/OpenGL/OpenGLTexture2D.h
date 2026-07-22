#pragma once

#include "Rendering/Texture2D.h"

namespace CgEngine {
    class OpenGLTexture2D : public Texture2D {
    public:
        OpenGLTexture2D() = default;
        OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);
        OpenGLTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);
        OpenGLTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering= MipMapFiltering::Anisotropic, TextureBorderColor borderColor = TextureBorderColor::OpaqueWhite);

        ~OpenGLTexture2D() override;

        OpenGLTexture2D(OpenGLTexture2D&& other) noexcept;
        OpenGLTexture2D& operator=(OpenGLTexture2D&& other) noexcept;

        OpenGLTexture2D(OpenGLTexture2D& other) = delete;
        OpenGLTexture2D& operator=(OpenGLTexture2D& other) = delete;

        uint32_t getWidth() const override;
        uint32_t getHeight() const override;
        TextureFormat getFormat() const override;

        uint32_t getOpenGLHandle() const;

    private:
        uint32_t id = ~0;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
    };
}
