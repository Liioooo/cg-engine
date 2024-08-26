#include "Framebuffer.h"
#include "Asserts.h"
#include "glad/glad.h"

namespace CgEngine {
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
                    if (format == FramebufferFormat::RGBA16F) {
                        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, specification.samples, GL_RGBA16F, specification.width, specification.height, GL_FALSE);
                    } else {
                        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, specification.samples, GL_RGBA8, specification.width, specification.height, GL_FALSE);
                    }
                    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D_MULTISAMPLE, colorAttachments.back(), 0);
                } else {
                    glGenTextures(1, &colorAttachments.back());
                    glBindTexture(GL_TEXTURE_2D, colorAttachments.back());
                    if (format == FramebufferFormat::RGBA16F) {
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, specification.width, specification.height, 0, GL_RGBA, GL_FLOAT, nullptr);
                    } else {
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, specification.width, specification.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                    }
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
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, specification.existingColorAttachment, specification.existingColorAttachmentLevel);
            colorAttachments.push_back(specification.existingColorAttachment);
        }

        if (specification.colorAttachments.empty() && !specification.useExistingColorAttachment) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
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

    void Framebuffer::setColorAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height) {
        if (!specification.useExistingColorAttachment) {
            for (const auto &item: colorAttachments) {
                glDeleteTextures(1, &item);
            }
            specification.colorAttachments.clear();
        }
        glNamedFramebufferTexture(id, GL_COLOR_ATTACHMENT0, attachment, level);
        specification.useExistingColorAttachment = true;
        specification.existingColorAttachment = attachment;
        specification.existingColorAttachmentLevel = level;
        specification.width = width;
        specification.height = height;
        colorAttachments.clear();
        colorAttachments.push_back(attachment);
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
