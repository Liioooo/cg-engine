#pragma once

#include "XMLFile.h"
#include "Resources/ResRef.h"
#include "Material.h"

namespace CgEngine {

    class PBRMaterial : public Material, public Resource {
    public:
        static PBRMaterial* createResource(const std::string& name);

        explicit PBRMaterial();

        void setAlbedoColor(glm::vec3 value);
        void setMetalness(float value);
        void setRoughness(float value);
        void setEmission(glm::vec3 value);
        void setEmissionTexture(ResRef<Texture2D> texture);
        void setAlbedoTexture(ResRef<Texture2D> texture);
        void setMetalnessTexture(ResRef<Texture2D> texture);
        void setRoughnessTexture(ResRef<Texture2D> texture);
        void setNormalTexture(ResRef<Texture2D> texture);

    private:
        ResRef<Texture2D> emissionTexture;
        ResRef<Texture2D> albedoTexture;
        ResRef<Texture2D> metalnessTexture;
        ResRef<Texture2D> roughnessTexture;
        ResRef<Texture2D> normalTexture;

        static inline XMLFile xmlMaterialFile;
    };

}
