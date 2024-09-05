#pragma once

#include "Material.h"

namespace CgEngine {

    class PBRMaterial : public Material, public Resource {
    public:
        static PBRMaterial* createResource(const std::string& name);

        explicit PBRMaterial();

    public:
        void setAlbedoColor(glm::vec3 value);
        void setMetalness(float value);
        void setRoughness(float value);
        void setEmission(glm::vec3 value);
        void setEmissionTexture(const Texture2D* texture);
        void setAlbedoTexture(const Texture2D* texture);
        void setMetalnessTexture(const Texture2D* texture);
        void setRoughnessTexture(const Texture2D* texture);
        void setNormalTexture(const Texture2D* texture);

    };

}
