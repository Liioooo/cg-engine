#pragma once

#include "Rendering/Framebuffer.h"
#include "Rendering/Attachment.h"

namespace CgEngine {

    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer() = default;
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        OpenGLFramebuffer(uint32_t width, uint32_t height, bool swapChainTarget = true);

        ~OpenGLFramebuffer() override;

        OpenGLFramebuffer(OpenGLFramebuffer&& other) noexcept;
        OpenGLFramebuffer& operator=(OpenGLFramebuffer&& other) noexcept;

        OpenGLFramebuffer(OpenGLFramebuffer& other) = delete;
        OpenGLFramebuffer& operator=(OpenGLFramebuffer& other) = delete;

        void recreate(uint32_t newWidth, uint32_t newHeight) override;

        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

        uint32_t getOpenGLHandle() const;
        bool hasStencilAttachment() const;


    private:
        uint32_t framebufferHandle = ~0;
        uint32_t width = 0;
        uint32_t height = 0;

        std::vector<FramebufferAttachment> colorAttachments;
        FramebufferAttachment depthStencilAttachment;

        void init();
    };
}
