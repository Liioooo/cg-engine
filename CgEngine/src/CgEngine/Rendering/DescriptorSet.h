#pragma once

#include "DescriptorSetLayout.h"
#include "Attachment.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "ImmutableShaderStorageBuffer.h"
#include "ShaderStorageBuffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"

namespace CgEngine {

    struct DescSetUBOBinding {
        uint32_t bindingPoint;
        const UniformBuffer* ubo;
    };

    struct DescSetSSBOBinding {
        uint32_t bindingPoint;
        const ShaderStorageBuffer* ssbo;
    };

    struct DescSetImmutableSSBOBinding {
        uint32_t bindingPoint;
        const ImmutableShaderStorageBuffer* ssbo;
    };

    struct DescSetTexture2DBinding {
        uint32_t bindingPoint;
        const Texture2D* texture;
    };

    struct DescSetTextureCubeBinding {
        uint32_t bindingPoint;
        const TextureCube* texture;
    };

    struct DescSetAttachmentTextureBinding {
        uint32_t bindingPoint;
        uint32_t layer = ~0;
        bool allLayers = true;
        const Attachment* attachment;
    };

    struct DescSetImageCubeBinding {
        uint32_t bindingPoint;
        uint32_t mipLevel = 0;
        ShaderImageAccess access;
        const TextureCube* texture;
    };

    struct DescSetAttachmentImageBinding {
        uint32_t bindingPoint;
        uint32_t layer = ~0;
        bool allLayers = true;
        ShaderImageAccess access;
        const Attachment* attachment;
    };

    struct DescSetVertexBufferSSBOBinding {
        uint32_t bindingPoint;
        const VertexBuffer* vertexBuffer;
    };

    struct DescriptorSetSpecification {
        const DescriptorSetLayout* layout;
        std::vector<DescSetUBOBinding> uboBindings;
        std::vector<DescSetSSBOBinding> ssboBindings;
        std::vector<DescSetImmutableSSBOBinding> immutableSsboBindings;
        std::vector<DescSetTexture2DBinding> texture2DBindings;
        std::vector<DescSetTextureCubeBinding> textureCubeBindings;
        std::vector<DescSetAttachmentTextureBinding> attachmentTextureBindings;
        std::vector<DescSetImageCubeBinding> imageCubeBindings;
        std::vector<DescSetAttachmentImageBinding> attachmentImageBindings;
        std::vector<DescSetVertexBufferSSBOBinding> vertexBufferSSBOBindings;
    };

    class DescriptorSet {
    public:
        DescriptorSet() = default;

        virtual ~DescriptorSet() = default;

        DescriptorSet(DescriptorSet&& other) noexcept = default;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept = default;

        DescriptorSet(DescriptorSet& other) = delete;
        DescriptorSet& operator=(DescriptorSet& other) = delete;

        virtual bool isReady() const = 0;
        virtual void recreate() =  0;
        virtual void reconfigure(const DescriptorSetSpecification& spec) = 0;
    };

}
