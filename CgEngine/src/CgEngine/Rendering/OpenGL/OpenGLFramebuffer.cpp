#include <Asserts.h>
#include "OpenGLFramebuffer.h"
#include "glad/glad.h"
#include "OpenGLAttachment.h"

namespace CgEngine {

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : width(spec.width), height(spec.height), colorAttachments(spec.colorAttachments), depthStencilAttachment(spec.depthStencilAttachment) {
        init();
    }

    OpenGLFramebuffer::OpenGLFramebuffer(uint32_t width, uint32_t height, bool swapChainTarget) : width(width), height(height) {
        CG_ASSERT(width > 0 && height > 0, "Framebuffer width and height must be greater than 0!")
        CG_ASSERT(swapChainTarget, "swapChainTarget must be true for this constructor!")
        framebufferHandle = 0;
    }

    OpenGLFramebuffer::~OpenGLFramebuffer() {
        if (framebufferHandle != ~0 && framebufferHandle != 0) {
            glDeleteFramebuffers(1, &framebufferHandle);
        }
    }

    OpenGLFramebuffer::OpenGLFramebuffer(OpenGLFramebuffer&& other) noexcept : Framebuffer(std::move(other)) {
        framebufferHandle = other.framebufferHandle;
        width = other.width;
        height = other.height;
        other.framebufferHandle = ~0;
    }

    OpenGLFramebuffer& OpenGLFramebuffer::operator=(OpenGLFramebuffer&& other) noexcept {
        if (this != &other) {
            Framebuffer::operator=(std::move(other));

            if (framebufferHandle != ~0 && framebufferHandle != 0) {
                glDeleteFramebuffers(1, &framebufferHandle);
            }
            framebufferHandle = other.framebufferHandle;
            width = other.width;
            height = other.height;
            other.framebufferHandle = ~0;
        }
        return *this;
    }

    void OpenGLFramebuffer::recreate(uint32_t newWidth, uint32_t newHeight) {
        if (framebufferHandle != ~0 && framebufferHandle != 0) {
            glDeleteFramebuffers(1, &framebufferHandle);
        }

        width = newWidth;
        height = newHeight;

        if (framebufferHandle != 0) {
            init();
        }
    }

    uint32_t OpenGLFramebuffer::getWidth() const {
        return width;
    }

    uint32_t OpenGLFramebuffer::getHeight() const {
        return height;
    }

    uint32_t OpenGLFramebuffer::getOpenGLHandle() const {
        CG_ASSERT(framebufferHandle != ~0, "Framebuffer is not initialized!")
        return framebufferHandle;
    }

    bool OpenGLFramebuffer::hasStencilAttachment() const  {
        CG_ASSERT(framebufferHandle != ~0, "Framebuffer is not initialized!")
        if (depthStencilAttachment.attachment) {
            auto* depthAttachmentGL = static_cast<const OpenGLAttachment*>(depthStencilAttachment.attachment);
            return depthAttachmentGL->getType() == AttachmentType::DepthStencil;
        }
    }

    void OpenGLFramebuffer::init() {
        CG_ASSERT(width > 0 && height > 0, "Framebuffer width and height must be greater than 0!")
        CG_ASSERT(colorAttachments.size() > 0 || depthStencilAttachment.attachment != nullptr, "At least one attachment (color or depth) must be provided!")

        glGenFramebuffers(1, &framebufferHandle);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);

        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(colorAttachments.size());

        for (size_t i = 0; i < colorAttachments.size(); i++) {
            const auto* attachment = static_cast<const OpenGLAttachment*>(colorAttachments[i].attachment);

            if (colorAttachments[i].allLayers) {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            } else {
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0, colorAttachments[i].layer);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
        }

        if (!drawBuffers.empty()) {
            glDrawBuffers(static_cast<int>(drawBuffers.size()), drawBuffers.data());
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }


        if (depthStencilAttachment.attachment) {
            auto* depthAttachmentGL = static_cast<const OpenGLAttachment*>(depthStencilAttachment.attachment);
            CG_ASSERT(depthAttachmentGL->getType() == AttachmentType::Depth || depthAttachmentGL->getType() == AttachmentType::DepthStencil, "DepthStencil attachment must be of type Depth or DepthStencil")

            if (depthAttachmentGL->getType() == AttachmentType::DepthStencil) {
                if (depthStencilAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0, depthStencilAttachment.layer);
                }
            } else {
                if (depthStencilAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0, depthStencilAttachment.layer);
                }
            }
        }

        CG_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!")

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
