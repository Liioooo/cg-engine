#pragma once

#include "XMLFile.h"
#include "Resources/ResRef.h"
#include "Material.h"

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

    struct PBRMaterialPushConstants {
        glm::vec3 albedoColor;
        float metalness;
        float roughness;
        glm::vec3 emission;
        bool useNormals;
    };

    class PBRMaterial : public Material {
    public:
        static PBRMaterial* createResource(const std::string& name);

        explicit PBRMaterial(PBRMaterialSpecification spec);

    private:
        PBRMaterialPushConstants pushConstantsData;
        ResRef<Texture2D> emissionTexture;
        ResRef<Texture2D> albedoTexture;
        ResRef<Texture2D> metalnessTexture;
        ResRef<Texture2D> roughnessTexture;
        ResRef<Texture2D> normalTexture;

        static inline XMLFile xmlMaterialFile;
    };

}
