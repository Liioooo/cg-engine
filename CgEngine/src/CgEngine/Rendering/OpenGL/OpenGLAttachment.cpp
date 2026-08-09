#include <Asserts.h>
#include "OpenGLAttachment.h"
#include "glad/glad.h"
#include "OpenGLHelpers.h"

namespace CgEngine {

    OpenGLAttachment::OpenGLAttachment(const AttachmentSpecification& spec) : usableAsTexture(spec.usableAsTexture), usableAsStorageImage(spec.usableAsStorageImage), type(spec.type), mipMapFiltering(spec.mipMapFiltering), textureWrap(spec.textureWrap), textureBorderColor(spec.textureBorderColor), layerCount(spec.layerCount), width(spec.width), height(spec.height) {
        GLenum target = (spec.layerCount > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

        glCreateTextures(target, 1, &attachmentHandle);
        glBindTexture(target, attachmentHandle);

        if (spec.type == AttachmentType::Depth) {
            if (spec.layerCount > 1) {
                glTexStorage3D(target, 1, GL_DEPTH_COMPONENT32F, spec.width, spec.height, spec.layerCount);
            } else {
                glTexStorage2D(target, 1, GL_DEPTH_COMPONENT32F, spec.width, spec.height);
            }
            depthFormat = DepthStencilAttachmentFormat::Depth32Float;
        } else if (spec.type == AttachmentType::DepthStencil) {
            if (spec.layerCount > 1) {
                glTexStorage3D(target, 1, GL_DEPTH32F_STENCIL8, spec.width, spec.height, spec.layerCount);
            } else {
                glTexStorage2D(target, 1, GL_DEPTH32F_STENCIL8, spec.width, spec.height);
            }
            depthFormat = DepthStencilAttachmentFormat::Depth32FloatStencil8;
        } else {
            if (spec.layerCount > 1) {
                glTexStorage3D(target, 1, OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(spec.type), spec.width, spec.height, spec.layerCount);
            } else {
                glTexStorage2D(target, 1, OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(spec.type), spec.width, spec.height);
            }
        }

        if (spec.usableAsTexture) {
            CG_ASSERT(spec.mipMapFiltering != MipMapFiltering::Trilinear, "Trilinear filtering is not supported for Attachments!")
            CG_ASSERT(spec.mipMapFiltering != MipMapFiltering::Anisotropic, "Anisotropic filtering is not supported for Attachments!")

            OpenGLHelpers::applyMipMapFiltering(spec.mipMapFiltering, GL_TEXTURE_2D);
            GLint wrapMode = OpenGLHelpers::getOpenGLWrapMode(spec.textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(spec.textureBorderColor)));
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        if (spec.usableAsTexture && spec.layerCount > 1) {
            layerViewHandles.resize(spec.layerCount);

            GLenum internalFormat = 0;

            if (spec.type == AttachmentType::Depth) {
                internalFormat = GL_DEPTH_COMPONENT32F;
            } else if (spec.type == AttachmentType::DepthStencil) {
                internalFormat = GL_DEPTH32F_STENCIL8;
            } else {
                internalFormat = OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(spec.type);
            }

            for (int layer = 0; layer < layerCount; ++layer) {
                GLuint viewHandle = 0;
                glGenTextures(1, &viewHandle);

                glTextureView(viewHandle, GL_TEXTURE_2D, attachmentHandle, internalFormat, 0, 1, layer, 1);

                OpenGLHelpers::applyMipMapFiltering(spec.mipMapFiltering, GL_TEXTURE_2D);
                GLint wrapMode = OpenGLHelpers::getOpenGLWrapMode(spec.textureWrap);
                glTextureParameteri(viewHandle, GL_TEXTURE_WRAP_S, wrapMode);
                glTextureParameteri(viewHandle, GL_TEXTURE_WRAP_T, wrapMode);
                glTextureParameterfv(viewHandle, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(spec.textureBorderColor)));


                layerViewHandles[layer] = viewHandle;
            }
        }
    }

    OpenGLAttachment::~OpenGLAttachment() {
        if (attachmentHandle != ~0) {
            glDeleteTextures(1, &attachmentHandle);
        }
        for (auto handle : layerViewHandles) {
            glDeleteTextures(1, &handle);
        }
    }

    OpenGLAttachment::OpenGLAttachment(OpenGLAttachment&& other) noexcept : Attachment(std::move(other)) {
        attachmentHandle = other.attachmentHandle;
        other.attachmentHandle = ~0;
        type = other.type;
        depthFormat = other.depthFormat;
        usableAsTexture = other.usableAsTexture;
        usableAsStorageImage = other.usableAsStorageImage;
        mipMapFiltering = other.mipMapFiltering;
        textureWrap = other.textureWrap;
        layerCount = other.layerCount;
        layerViewHandles = std::move(other.layerViewHandles);
    }

    OpenGLAttachment& OpenGLAttachment::operator=(OpenGLAttachment&& other) noexcept {
        if (this != &other) {
            Attachment::operator=(std::move(other));

            if (attachmentHandle != ~0) {
                glDeleteTextures(1, &attachmentHandle);
            }
            for (auto handle : layerViewHandles) {
                glDeleteTextures(1, &handle);
            }

            attachmentHandle = other.attachmentHandle;
            other.attachmentHandle = ~0;
            type = other.type;
            depthFormat = other.depthFormat;
            usableAsTexture = other.usableAsTexture;
            usableAsStorageImage = other.usableAsStorageImage;
            mipMapFiltering = other.mipMapFiltering;
            textureWrap = other.textureWrap;
            layerCount = other.layerCount;
            layerViewHandles = std::move(other.layerViewHandles);
        }
        return *this;
    }

    AttachmentType OpenGLAttachment::getType() const {
        return type;
    }

    DepthStencilAttachmentFormat OpenGLAttachment::getDepthStencilAttachmentFormat() const {
        return depthFormat;
    }

    bool OpenGLAttachment::isUsableAsTexture() const {
        return usableAsTexture;
    }

    bool OpenGLAttachment::isUsableAsStorageImage() const {
        return usableAsStorageImage;
    }

    uint32_t OpenGLAttachment::getLayerCount() const {
        return layerCount;
    }

    uint32_t OpenGLAttachment::getWidth() const {
        return width;
    }

    uint32_t OpenGLAttachment::getHeight() const {
        return height;
    }

    void OpenGLAttachment::resize(uint32_t newWidth, uint32_t newHeight) {
        if (attachmentHandle != ~0) {
            glDeleteTextures(1, &attachmentHandle);
        }
        for (auto handle : layerViewHandles) {
            glDeleteTextures(1, &handle);
        }

        width = newWidth;
        height = newHeight;

        GLenum target = (layerCount > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

        glCreateTextures(target, 1, &attachmentHandle);
        glBindTexture(target, attachmentHandle);

        if (type == AttachmentType::Depth) {
            if (layerCount > 1) {
                glTexStorage3D(target, 1, GL_DEPTH_COMPONENT32F, newWidth, newHeight, layerCount);
            } else {
                glTexStorage2D(target, 1, GL_DEPTH_COMPONENT32F, newWidth, newHeight);
            }
        } else if (type == AttachmentType::DepthStencil) {
            if (layerCount > 1) {
                glTexStorage3D(target, 1, GL_DEPTH32F_STENCIL8, newWidth, newHeight, layerCount);
            } else {
                glTexStorage2D(target, 1, GL_DEPTH32F_STENCIL8, newWidth, newHeight);
            }
        } else {
            if (layerCount > 1) {
                glTexStorage3D(target, 1, OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(type), newWidth, newHeight, layerCount);
            } else {
                glTexStorage2D(target, 1, OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(type), newWidth, newHeight);
            }
        }

        if (usableAsTexture) {
            CG_ASSERT(mipMapFiltering != MipMapFiltering::Trilinear, "Trilinear filtering is not supported for Attachments!")

            OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
            GLint wrapMode = OpenGLHelpers::getOpenGLWrapMode(textureWrap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(textureBorderColor)));
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        if (usableAsTexture && layerCount > 1) {
            layerViewHandles.resize(layerCount);

            GLenum internalFormat = OpenGLHelpers::attachmentTypeToOpenGLInternalFormat(type);

            for (int layer = 0; layer < layerCount; ++layer) {
                GLuint viewHandle = 0;
                glGenTextures(1, &viewHandle);
                glTextureView(viewHandle, GL_TEXTURE_2D, attachmentHandle, internalFormat, 0, 1, layer, 1);
                glBindTexture(GL_TEXTURE_2D, viewHandle);

                OpenGLHelpers::applyMipMapFiltering(mipMapFiltering, GL_TEXTURE_2D);
                GLint wrapMode = OpenGLHelpers::getOpenGLWrapMode(textureWrap);
                glTextureParameteri(viewHandle, GL_TEXTURE_WRAP_S, wrapMode);
                glTextureParameteri(viewHandle, GL_TEXTURE_WRAP_T, wrapMode);
                glTextureParameterfv(viewHandle, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGLHelpers::textureBorderColorToGLMVec3(textureBorderColor)));

                layerViewHandles[layer] = viewHandle;
            }
        }
    }

    uint32_t OpenGLAttachment::getOpenGLHandle() const {
        CG_ASSERT(attachmentHandle != ~0, "Attachment::getOpenGLHandle: Attachment has not been created properly.")
        return attachmentHandle;
    }

    uint32_t OpenGLAttachment::getOpenGLLayerViewHandle(uint32_t layer) const {
        CG_ASSERT(usableAsTexture, "Attachment is not usable as texture!")
        CG_ASSERT(layer < layerCount, "Attachment::getOpenGLLayerViewHandle: Layer index out of bounds.")
        CG_ASSERT(!layerViewHandles.empty(), "Attachment::getOpenGLLayerViewHandle: Attachment has not been created properly.")
        return layerViewHandles[layer];
    }

    OpenGLAttachmentState OpenGLAttachment::getState() const {
        return state;
    }

    void OpenGLAttachment::setState(OpenGLAttachmentState newState) {
        state = newState;
    }
}
