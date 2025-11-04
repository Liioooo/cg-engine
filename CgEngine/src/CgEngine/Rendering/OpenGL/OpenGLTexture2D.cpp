#include <FileSystem.h>
#include <Asserts.h>
#include "OpenGLTexture2D.h"
#include "glad/glad.h"
#include "OpenGLHelpers.h"
#include "stbi_image.h"

namespace CgEngine {

    OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, MipMapFiltering mipMapFiltering) : format(format), width(width), height(height) {
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
        GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
        GLenum glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, nullptr);
    }


    OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering) : format(format), width(width), height(height) {
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
        GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
        GLenum glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, data);

        if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering) {
        int loadWidth, loadHeight, channels;

        CG_ASSERT(FileSystem::checkFileExists(path), "Texture2D: " + path.string() + " does not exist!")

        std::string pathString = path.string();

        unsigned char* data;

        if (stbi_is_hdr(pathString.c_str())) {
            stbi_info(pathString.c_str(), &loadWidth, &loadHeight, &channels);
            if (channels <= 3) {
                data = (unsigned char*)(stbi_loadf(pathString.c_str(), &loadWidth, &loadHeight, &channels, STBI_rgb));
                format = TextureFormat::Float32;
            } else {
                data = (unsigned char*)(stbi_loadf(pathString.c_str(), &loadWidth, &loadHeight, &channels, STBI_rgb_alpha));
                format = TextureFormat::Float32A;
            }
        } else {
            stbi_info(pathString.c_str(), &loadWidth, &loadHeight, &channels);
            if (channels <= 3) {
                data = stbi_load(pathString.c_str(), &loadWidth, &loadHeight, &channels, STBI_rgb);
                format = TextureFormat::RGB;
            } else {
                data = stbi_load(pathString.c_str(), &loadWidth, &loadHeight, &channels, STBI_rgb_alpha);
                format = TextureFormat::RGBA;
            }
        }

        if (!data) {
            return;
        }

        width = loadWidth;
        height = loadHeight;

        if (srgb) {
            glCreateTextures(GL_TEXTURE_2D, 1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
            GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

            GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
            GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
            glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_SRGB : GL_SRGB_ALPHA, width, height, 0, glFormat, type, data);

            if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        } else {
            glCreateTextures(GL_TEXTURE_2D, 1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
            GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

            GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
            GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
            GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, data);

            if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }

        stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering) {
        int loadWidth, loadHeight, channels;
        unsigned char* data;

        if (stbi_is_hdr_from_memory(buffer, bufferLen)) {
            stbi_info_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels);
            if (channels <= 3) {
                data = (unsigned char*)(stbi_loadf_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels, STBI_rgb));
                format = TextureFormat::Float32;
            } else {
                data = (unsigned char*)(stbi_loadf_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels, STBI_rgb_alpha));
                format = TextureFormat::Float32A;
            }
        } else {
            stbi_info_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels);
            if (channels <= 3) {
                data = stbi_load_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels, STBI_rgb);
                format = TextureFormat::RGB;
            } else {
                data = stbi_load_from_memory(buffer, bufferLen, &loadWidth, &loadHeight, &channels, STBI_rgb_alpha);
                format = TextureFormat::RGBA;
            }
        }

        if (!data) {
            return;
        }

        width = loadWidth;
        height = loadHeight;

        if (srgb) {
            glCreateTextures(GL_TEXTURE_2D, 1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
            GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

            GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
            GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
            glTexImage2D(GL_TEXTURE_2D, 0, channels == 3 ? GL_SRGB : GL_SRGB_ALPHA, width, height, 0, glFormat, type, data);

            if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        } else {
            glCreateTextures(GL_TEXTURE_2D, 1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
            GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);

            GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
            GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
            GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, data);

            if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }

        stbi_image_free(data);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        if (id != ~0) {
            glDeleteTextures(1, &id);
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(OpenGLTexture2D&& other) noexcept : Texture2D(std::move(other)) {
        id = other.id;
        width = other.width;
        height = other.height;
        format = other.format;

        other.id = ~0;
        other.width = 0;
        other.height = 0;
    }

    OpenGLTexture2D& OpenGLTexture2D::operator=(OpenGLTexture2D&& other) noexcept {
        if (this != &other) {
            Texture2D::operator=(std::move(other));

            if (id != ~0) {
                glDeleteTextures(1, &id);
            }

            id = other.id;
            width = other.width;
            height = other.height;
            format = other.format;

            other.id = ~0;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    uint32_t OpenGLTexture2D::getWidth() const {
        return width;
    }

    uint32_t OpenGLTexture2D::getHeight() const {
        return height;
    }

    TextureFormat OpenGLTexture2D::getFormat() const {
        return format;
    }

    void OpenGLTexture2D::bufferSubData(int x, int y, int w, int h, const void* data, int alignment) {
        CG_ASSERT(id != ~0, "OpenGLTexture2D: Attempting to buffer sub data of a moved-from or uninitialized texture!")

        glBindTexture(GL_TEXTURE_2D, id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

        GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTextureSubImage2D(id, 0, x, y, w, h, glFormat, type, data);
    }

    uint32_t OpenGLTexture2D::getOpenGLHandle() const {
        CG_ASSERT(id != ~0, "OpenGLTexture2D: Attempting to get OpenGL handle of a moved-from or uninitialized texture!")
        return id;
    }

}
