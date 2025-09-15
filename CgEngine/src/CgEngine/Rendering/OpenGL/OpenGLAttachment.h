#pragma once

#include "Rendering/Attachment.h"

namespace CgEngine {

    class OpenGLAttachment : public Attachment {
    public:
        OpenGLAttachment() = default;
        explicit OpenGLAttachment(const AttachmentSpecification& spec);

        ~OpenGLAttachment() override;

        OpenGLAttachment(OpenGLAttachment&& other) noexcept;
        OpenGLAttachment& operator=(OpenGLAttachment&& other) noexcept;

        OpenGLAttachment(OpenGLAttachment& other) = delete;
        OpenGLAttachment& operator=(OpenGLAttachment& other) = delete;

        AttachmentType getType() const override;
        DepthAttachmentFormat getDepthAttachmentFormat() const override;
        bool isUsableAsTexture() const override;
        uint32_t getLayerCount() const override;

        void resize(uint32_t newWidth, uint32_t newHeight) override;

        uint32_t getOpenGLHandle() const;
        uint32_t getOpenGLLayerViewHandle(uint32_t layer) const;

    private:
        uint32_t attachmentHandle = ~0;
        DepthAttachmentFormat depthFormat;
        AttachmentType type;
        bool usableAsTexture = false;
        TextureWrap textureWrap = TextureWrap::Clamp;
        MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear;
        TextureBorderColor textureBorderColor = TextureBorderColor::OpaqueBlack;
        uint32_t layerCount;
        std::vector<uint32_t> layerViewHandles;
    };
}
