#pragma once

#include "Attachment.h"

namespace CgEngine {

    struct FramebufferAttachment {
        Attachment* attachment = nullptr;
        uint32_t layer = ~0;
        bool allLayers = true;

        FramebufferAttachment() = default;
        FramebufferAttachment(Attachment* attachment, uint32_t layer = ~0, bool allLayers = true) : attachment(attachment), layer(layer), allLayers(allLayers) {}
    };

    struct FramebufferSpecification {
        uint32_t width;
        uint32_t height;
        std::vector<FramebufferAttachment> colorAttachments;
        FramebufferAttachment depthStencilAttachment;
    };

    class Framebuffer {
    public:
        Framebuffer() = default;

        virtual ~Framebuffer() = default;

        Framebuffer(Framebuffer&& other) noexcept = default;
        Framebuffer& operator=(Framebuffer&& other) noexcept = default;

        Framebuffer(Framebuffer& other) = delete;
        Framebuffer& operator=(Framebuffer& other) = delete;

        virtual void recreate(uint32_t newWidth, uint32_t newHeight) = 0;

        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

        virtual const std::vector<FramebufferAttachment>& getColorFramebufferAttachments() const = 0;
        virtual const FramebufferAttachment& getDepthStencilFramebufferAttachment() const = 0;

        virtual bool isReady() const = 0;
    };

}
