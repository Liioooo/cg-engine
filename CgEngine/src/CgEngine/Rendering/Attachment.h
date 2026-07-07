#pragma once

#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    struct AttachmentSpecification {
        AttachmentType type;
        uint32_t width;
        uint32_t height;
        bool usableAsTexture = false;
        bool usableAsStorageImage = false;
        TextureWrap textureWrap = TextureWrap::Clamp;
        MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear;
        TextureBorderColor textureBorderColor = TextureBorderColor::OpaqueBlack;
        uint32_t layerCount = 1;
    };

    class Attachment {
    public:
        Attachment() = default;

        virtual ~Attachment() = default;

        Attachment(Attachment&& other) noexcept = default;
        Attachment& operator=(Attachment&& other) noexcept = default;

        Attachment(Attachment& other) = delete;
        Attachment& operator=(Attachment& other) = delete;

        virtual AttachmentType getType() const = 0;
        virtual DepthStencilAttachmentFormat getDepthStencilAttachmentFormat() const = 0;
        virtual bool isUsableAsTexture() const = 0;
        virtual bool isUsableAsStorageImage() const = 0;
        virtual uint32_t getLayerCount() const = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

        virtual void resize(uint32_t newWidth, uint32_t newHeight) = 0;
    };

}
