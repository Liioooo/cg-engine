#include "OpenGLTextureCube.h"
#include "glad/glad.h"
#include "Asserts.h"
#include "OpenGLHelpers.h"
#include "Rendering/Helpers.h"

namespace CgEngine {

    OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering) : format(format), width(width), height(height) {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureHandle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format, false);

        int levels = 1;
        if (mipMapFiltering == MipMapFiltering::Trilinear) {
            levels = Helpers::calculateMipCount(width, height);
        }

        glTextureStorage2D(textureHandle, levels, internalFormat, width, height);

        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering) : format(format), width(width), height(height) {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureHandle);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
        OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        GLint internalFormat = OpenGLHelpers::getOpenGLTextureInternalFormat(format, false);
        GLenum glFormat = OpenGLHelpers::getOpenGLTextureFormat(format);
        GLenum type = OpenGLHelpers::getOpenGLTextureType(format);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, width, height, 0, glFormat, type, data);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, width, height, 0, glFormat, type, data);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, width, height, 0, glFormat, type, data);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, width, height, 0, glFormat, type, data);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, width, height, 0, glFormat, type, data);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, width, height, 0, glFormat, type, data);

        if (mipMapFiltering == MipMapFiltering::Trilinear) {
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
    }

    OpenGLTextureCube::~OpenGLTextureCube() {
        if (textureHandle != ~0) {
            glDeleteTextures(1, &textureHandle);
        }
    }

    OpenGLTextureCube::OpenGLTextureCube(OpenGLTextureCube&& other) noexcept {
        textureHandle = other.textureHandle;
        width = other.width;
        height = other.height;
        format = other.format;

        other.textureHandle = ~0;
        other.width = 0;
        other.height = 0;
    }

    OpenGLTextureCube& OpenGLTextureCube::operator=(OpenGLTextureCube&& other) noexcept {
        if (this != &other) {
            TextureCube::operator=(std::move(other));

            if (textureHandle != ~0) {
                glDeleteTextures(1, &textureHandle);
            }

            textureHandle = other.textureHandle;
            width = other.width;
            height = other.height;
            format = other.format;

            other.textureHandle = ~0;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    bool OpenGLTextureCube::isReady() const {
        return textureHandle != ~0;
    }

    uint32_t OpenGLTextureCube::getWidth() const {
        return width;
    }

    uint32_t OpenGLTextureCube::getHeight() const {
        return height;
    }

    TextureFormat OpenGLTextureCube::getFormat() const {
        return format;
    }

    void OpenGLTextureCube::generateMipMaps() {
        CG_ASSERT(isReady(), "OpenGLTextureCube::generateMipMaps: Texture is not ready!")
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    uint32_t OpenGLTextureCube::getOpenGLHandle() const {
        CG_ASSERT(isReady(), "OpenGLTextureCube::getOpenGLHandle: Texture is not ready!")
        return textureHandle;
    }

}
