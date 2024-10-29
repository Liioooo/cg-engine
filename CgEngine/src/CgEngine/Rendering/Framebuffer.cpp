#include "Framebuffer.h"
#include "Asserts.h"
#include "glad/glad.h"

namespace CgEngine {
    int FramebufferUtils::getOpenGLFramebufferInternalFormat(CgEngine::FramebufferFormat format) {
        switch (format) {
            case FramebufferFormat::RGB8:        return GL_RGB8;
            case FramebufferFormat::RGBA8:       return GL_RGBA8;
            case FramebufferFormat::RGBA16F:     return GL_RGBA16F;
            case FramebufferFormat::RGB16F:      return GL_RGB16F;
        }
        return 0;
    }

    Framebuffer::Framebuffer(FramebufferSpecification spec) : specification(std::move(spec)) {
        resize(specification.width, specification.height, true);
    }

    Framebuffer::~Framebuffer() {
        if (!specification.screenTarget) {
            glDeleteFramebuffers(1, &id);
            if (!specification.useExistingColorAttachment) {
                for (const auto &item: colorAttachments) {
                    glDeleteTextures(1, &item);
                }
            }
            if ((specification.hasDepthStencilAttachment || specification.hasDepthAttachment) && (!specification.useExistingDepthAttachment || !specification.useExistingDepthStencilAttachment)) {
                glDeleteTextures(1, &depthAttachment);
            }
        }
    }

    void Framebuffer::bind() {
        if (specification.screenTarget) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, id);
        }
        glViewport(0, 0, specification.width, specification.height);
    }

    void Framebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::resize(uint32_t width, uint32_t height, bool forceRecreate) {
        if (!forceRecreate && (specification.width == width && specification.height == height)) {
            return;
        }

        specification.width = width;
        specification.height = height;

        if (specification.screenTarget) {
            return;
        }

        if (!forceRecreate && (specification.useExistingColorAttachment || specification.colorAttachments.empty()) && ((specification.useExistingDepthAttachment || specification.useExistingDepthStencilAttachment) || !(specification.hasDepthAttachment || specification.hasDepthStencilAttachment))) {
            return;
        }

        if (id) {
            glDeleteFramebuffers(1, &id);
            if (!specification.useExistingColorAttachment) {
                for (const auto &item: colorAttachments) {
                    glDeleteTextures(1, &item);
                }
                colorAttachments.clear();
            }
            if ((specification.hasDepthStencilAttachment || specification.hasDepthAttachment) && (!specification.useExistingDepthAttachment || !specification.useExistingDepthStencilAttachment)) {
                glDeleteTextures(1, &depthAttachment);
            }
        }

        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        bool multisample = specification.samples > 1;

        CG_ASSERT(!specification.useExistingColorAttachment || specification.colorAttachments.empty(), "Already using existing Color Attachment")
        CG_ASSERT(!(specification.hasDepthStencilAttachment && specification.hasDepthAttachment), "Framebuffer can't have 2 Depth Attachments")
        CG_ASSERT(!(specification.hasDepthStencilAttachment || specification.hasDepthAttachment) || !(specification.useExistingDepthAttachment || specification.useExistingDepthStencilAttachment), "Already using existing Depth Attachment")

        if (!specification.useExistingColorAttachment) {
            uint32_t index = 0;
            for (const auto &format: specification.colorAttachments) {

                colorAttachments.push_back(0);

                if (multisample) {
                    glGenTextures(1, &colorAttachments.back());
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorAttachments.back());
                    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, specification.samples, FramebufferUtils::getOpenGLFramebufferInternalFormat(format), specification.width, specification.height, GL_FALSE);
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D_MULTISAMPLE, colorAttachments.back(), 0);
                } else {
                    glGenTextures(1, &colorAttachments.back());
                    glBindTexture(GL_TEXTURE_2D, colorAttachments.back());
                    glTexStorage2D(GL_TEXTURE_2D, 1, FramebufferUtils::getOpenGLFramebufferInternalFormat(format), specification.width, specification.height);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, colorAttachments.back(), 0);
                }

                index++;
            }
        } else {
            uint32_t index = 0;
            for (const auto& item: specification.existingColorAttachments) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, item, specification.existingColorAttachmentLevel);
                colorAttachments.push_back(item);
                index++;
            }
        }

        if (specification.colorAttachments.empty() && !specification.useExistingColorAttachment) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        } else {
            std::vector<GLenum> drawBuffers;
            drawBuffers.reserve(colorAttachments.size());
            for (uint32_t i = 0; i < colorAttachments.size(); i++) {
                drawBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);
            }
            glDrawBuffers(colorAttachments.size(), drawBuffers.data());
        }

        if (specification.hasDepthStencilAttachment) {
            if (multisample) {
                glGenTextures(1, &depthAttachment);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthAttachment);
                glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, specification.samples, GL_DEPTH24_STENCIL8, specification.width, specification.height, GL_FALSE);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            } else {
                glGenTextures(1, &depthAttachment);
                glBindTexture(GL_TEXTURE_2D, depthAttachment);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, specification.width, specification.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachment, 0);
        }

        if (specification.hasDepthAttachment) {
            if (multisample) {
                glGenTextures(1, &depthAttachment);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthAttachment);
                glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, specification.samples, GL_DEPTH_COMPONENT, specification.width, specification.height, GL_FALSE);
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            } else {
                glGenTextures(1, &depthAttachment);
                glBindTexture(GL_TEXTURE_2D, depthAttachment);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, specification.width, specification.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachment, 0);
        }

        if (specification.useExistingDepthStencilAttachment) {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, specification.existingDepthAttachment, specification.existingDepthAttachmentLevel);
            depthAttachment = specification.existingDepthAttachment;
        }

        if (specification.useExistingDepthAttachment) {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, specification.existingDepthAttachment, specification.existingDepthAttachmentLevel);
            depthAttachment = specification.existingDepthAttachment;
        }

        CG_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::setColorAttachments(const std::vector<uint32_t>& attachments, uint32_t level, uint32_t width, uint32_t height) {
        if (!specification.useExistingColorAttachment) {
            for (const auto &item: colorAttachments) {
                glDeleteTextures(1, &item);
            }
            specification.colorAttachments.clear();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, id);

        uint32_t index = 0;
        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(colorAttachments.size());
        for (const auto& item: attachments) {
            glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0 + index, item, level);
            drawBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + index);
            index++;
        }
        glDrawBuffers(colorAttachments.size(), drawBuffers.data());

        specification.useExistingColorAttachment = true;
        specification.existingColorAttachments = attachments;
        specification.existingColorAttachmentLevel = level;
        specification.width = width;
        specification.height = height;
        colorAttachments = attachments;
    }

    void Framebuffer::setDepthAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height) {
        if (specification.hasDepthStencilAttachment || specification.hasDepthAttachment) {
            specification.hasDepthStencilAttachment = false;
            specification.hasDepthAttachment = false;
            glDeleteTextures(1, &depthAttachment);
        }
        glNamedFramebufferTexture(id, GL_DEPTH_ATTACHMENT, attachment, level);
        specification.existingDepthAttachmentLevel = level;
        depthAttachment = attachment;
        specification.existingDepthAttachment = attachment;
        specification.useExistingDepthAttachment = true;
        specification.useExistingDepthStencilAttachment = false;
        specification.width = width;
        specification.height = height;
    }

    void Framebuffer::setDepthStencilAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height) {
        if (specification.hasDepthStencilAttachment || specification.hasDepthAttachment) {
            specification.hasDepthStencilAttachment = false;
            specification.hasDepthAttachment = false;
            glDeleteTextures(1, &depthAttachment);
        }
        glNamedFramebufferTexture(id, GL_DEPTH_STENCIL_ATTACHMENT, attachment, level);
        specification.existingDepthAttachmentLevel = level;
        depthAttachment = attachment;
        specification.existingDepthAttachment = attachment;
        specification.useExistingDepthStencilAttachment = true;
        specification.useExistingDepthAttachment = false;
        specification.width = width;
        specification.height = height;
    }

    uint32_t Framebuffer::getRendererId() const {
        return id;
    }

    uint32_t Framebuffer::getColorAttachmentRendererId(size_t index) const {
        return colorAttachments[index];
    }

    uint32_t Framebuffer::getDepthAttachmentRendererId() const {
        return depthAttachment;
    }

    const FramebufferSpecification& Framebuffer::getSpecification() {
        return specification;
    }
}
