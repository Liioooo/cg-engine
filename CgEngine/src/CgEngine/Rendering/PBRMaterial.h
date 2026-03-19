#pragma once

#include "DescriptorSet.h"
#include "Texture2D.h"
#include "XMLFile.h"
#include "Resources/ResRef.h"
#include "UniformBuffer.h"
#include "Uuid.h"

namespace CgEngine {

    struct PBRMaterialSpecification {
        glm::vec3 albedoColor = {1.0f, 1.0f, 1.0f};
        float metalness = 0.0f;
        float roughness = 1.0f;
        glm::vec3 emission = {0.0f, 0.0f, 0.0f};
        ResRef<Texture2D> emissionTexture;
        ResRef<Texture2D> albedoTexture;
        ResRef<Texture2D> metalnessTexture;
        ResRef<Texture2D> roughnessTexture;
        ResRef<Texture2D> normalTexture;
    };

    struct PBRMaterialData {
        alignas(16) glm::vec3 albedoColor;
        float metalness;
        float roughness;
        float _padding_0[2];
        alignas(16) glm::vec3 emission;
        uint32_t useNormals;
        float _padding_1[3];
    };

    class PBRMaterial {
    public:
        static PBRMaterial* createResource(const std::string& name);

        explicit PBRMaterial(PBRMaterialSpecification spec);
        ~PBRMaterial();

        const Uuid& getUuid() const;

        bool operator ==(const PBRMaterial& other) const;

        DescriptorSet* getDescriptorSet() const;

    private:
        UniformBuffer* materialBuffer;
        ResRef<Texture2D> emissionTexture;
        ResRef<Texture2D> albedoTexture;
        ResRef<Texture2D> metalnessTexture;
        ResRef<Texture2D> roughnessTexture;
        ResRef<Texture2D> normalTexture;

        DescriptorSet* descriptorSet = nullptr;
        Uuid uuid;

        static inline XMLFile xmlMaterialFile;
    };

}
