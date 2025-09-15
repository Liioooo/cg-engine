#include <Asserts.h>
#include "OpenGLFramebuffer.h"
#include "glad/glad.h"
#include "OpenGLAttachment.h"

namespace CgEngine {

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : width(spec.width), height(spec.height) {
        init(spec);
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

    void OpenGLFramebuffer::recreate(const CgEngine::FramebufferSpecification& spec) {
        if (framebufferHandle != ~0 && framebufferHandle != 0) {
            glDeleteFramebuffers(1, &framebufferHandle);
        }

        width = spec.width;
        height = spec.height;
        init(spec);
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

    void OpenGLFramebuffer::init(const FramebufferSpecification& spec) {
        CG_ASSERT(spec.width > 0 && spec.height > 0, "Framebuffer width and height must be greater than 0!")
        CG_ASSERT(spec.colorAttachments.size() > 0 || spec.depthAttachment.attachment != nullptr, "At least one attachment (color or depth) must be provided!")

        glGenFramebuffers(1, &framebufferHandle);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferHandle);

        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(spec.colorAttachments.size());

        for (size_t i = 0; i < spec.colorAttachments.size(); i++) {
            const auto* attachment = static_cast<const OpenGLAttachment*>(spec.colorAttachments[i].attachment);

            if (spec.colorAttachments[i].allLayers) {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            } else {
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0, spec.colorAttachments[i].layer);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
        }

        if (!drawBuffers.empty()) {
            glDrawBuffers(static_cast<int>(drawBuffers.size()), drawBuffers.data());
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }


        if (spec.depthAttachment.attachment) {
            auto* depthAttachmentGL = static_cast<const OpenGLAttachment*>(spec.depthAttachment.attachment);
            CG_ASSERT(depthAttachmentGL->getType() == AttachmentType::Depth || depthAttachmentGL->getType() == AttachmentType::DepthStencil, "Depth attachment must be of type Depth or DepthStencil")

            if (depthAttachmentGL->getType() == AttachmentType::DepthStencil) {
                if (spec.depthAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0, spec.depthAttachment.layer);
                }
            } else {
                if (spec.depthAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachmentGL->getOpenGLHandle(), 0, spec.depthAttachment.layer);
                }
            }
        }

        CG_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!")

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
