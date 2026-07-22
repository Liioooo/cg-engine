#include <Asserts.h>
#include "OpenGLTexture2D.h"
#include "glad/glad.h"
#include "OpenGLHelpers.h"
#include "Rendering/Helpers.h"

namespace CgEngine {

    OpenGLTexture2D::OpenGLTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) : format(format), width(width), height(height) {
        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
        GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(borderColor)));

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
        GLenum glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, data);

        if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
       auto loadData = loadTextureDataFromFile(path, srgb);

        if (!loadData.data) {
            return;
        }

        format = loadData.format;
        width = loadData.width;
        height = loadData.height;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
        GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(borderColor)));

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
        GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, loadData.data);

        if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        Helpers::freeImageData(loadData.data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        auto loadData = loadTextureDataFromMemory(buffer, bufferLen, srgb);

        if (!loadData.data) {
            return;
        }

        format = loadData.format;
        width = loadData.width;
        height = loadData.height;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
        GLint textureWrap = OpenGLHelpers::getOpenGLWrapMode(wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(borderColor)));

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format);
        GLint glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, glFormat, type, loadData.data);

        if (mipMapFiltering == MipMapFiltering::Trilinear || mipMapFiltering == MipMapFiltering::Anisotropic) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        Helpers::freeImageData(loadData.data);
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

    uint32_t OpenGLTexture2D::getOpenGLHandle() const {
        CG_ASSERT(id != ~0, "OpenGLTexture2D: Attempting to get OpenGL handle of a moved-from or uninitialized texture!")
        return id;
    }

}
