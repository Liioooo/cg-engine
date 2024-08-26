#pragma once

namespace CgEngine {

    enum class FramebufferFormat {
        RGBA8, RGBA16F
    };

    struct FramebufferSpecification {
        uint32_t width;
        uint32_t height;
        glm::vec4 clearColor;
        bool hasDepthStencilAttachment = true; // creates a new Texture and attaches it, if true, else does nothing
        bool hasDepthAttachment = false; // creates a new Texture and attaches it, if true, else does nothing
        bool useExistingDepthStencilAttachment = false; // attaches 'existingDepthAttachment' as DepthStencilAttachment
        bool useExistingDepthAttachment = false; // attaches 'existingDepthAttachment' as DepthAttachment
        uint32_t existingDepthAttachment;
        uint32_t existingDepthAttachmentLevel = 0;
        std::vector<FramebufferFormat> colorAttachments; // creates new Textures and attaches them
        bool useExistingColorAttachment = false; // attaches 'existingColorAttachment' as ColorAttachment0
        uint32_t existingColorAttachment;
        uint32_t existingColorAttachmentLevel = 0;
        uint32_t samples = 1;
        bool screenTarget = false;
    };

    class Framebuffer {
    public:
        explicit Framebuffer(FramebufferSpecification spec);
        ~Framebuffer();

        void bind();
        void unbind();
        void resize(uint32_t width, uint32_t height, bool forceRecreate);
        void setColorAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height);
        void setDepthAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height);
        void setDepthStencilAttachment(uint32_t attachment, uint32_t level, uint32_t width, uint32_t height);
        uint32_t getRendererId() const;
        uint32_t getColorAttachmentRendererId(size_t index) const;
        uint32_t getDepthAttachmentRendererId() const;
        const FramebufferSpecification& getSpecification();

    private:
        FramebufferSpecification specification;
        uint32_t id = 0;
        std::vector<uint32_t> colorAttachments;
        uint32_t depthAttachment = 0;
    };

}
