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
        const UniformBuffer* ubo = nullptr;
    };

    struct DescSetSSBOBinding {
        uint32_t bindingPoint;
        const ShaderStorageBuffer* ssbo = nullptr;
    };

    struct DescSetImmutableSSBOBinding {
        uint32_t bindingPoint;
        const ImmutableShaderStorageBuffer* ssbo = nullptr;
    };

    struct DescSetTexture2DBinding {
        uint32_t bindingPoint;
        const Texture2D* texture = nullptr;
        std::vector<const Texture2D*> textureArray{};
    };

    struct DescSetTextureCubeBinding {
        uint32_t bindingPoint;
        const TextureCube* texture = nullptr;
        std::vector<const TextureCube*> textureArray{};
    };

    struct DescSetAttachmentTextureBinding {
        uint32_t bindingPoint;
        uint32_t layer = ~0;
        bool allLayers = true;
        Attachment* attachment = nullptr;
    };

    struct DescSetAttachmentImageBinding {
        uint32_t bindingPoint;
        uint32_t layer = ~0;
        bool allLayers = true;
        ShaderStorageAccess access;
        Attachment* attachment = nullptr;
    };

    struct DescSetVertexBufferSSBOBinding {
        uint32_t bindingPoint;
        VertexBuffer* vertexBuffer = nullptr;
        ShaderStorageAccess access;
    };

    struct DescriptorSetSpecification {
        const DescriptorSetLayout* layout = nullptr;
        std::vector<DescSetUBOBinding> uboBindings;
        std::vector<DescSetSSBOBinding> ssboBindings;
        std::vector<DescSetImmutableSSBOBinding> immutableSsboBindings;
        std::vector<DescSetTexture2DBinding> texture2DBindings;
        std::vector<DescSetTextureCubeBinding> textureCubeBindings;
        std::vector<DescSetAttachmentTextureBinding> attachmentTextureBindings;
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
