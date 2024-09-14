#pragma once

#include "Resources/MeshVertices.h"
#include "Rendering/PBRMaterial.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct MeshRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        std::string material;
        bool castShadows = true;
        std::vector<std::string> meshNodes;

        void verifyParams() const;
    };

    class MeshRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, MeshRendererComponentParams& params);

        ResRef<MeshVertices> getMeshVertices();
        ResRef<PBRMaterial> getMaterial();
        bool getCastShadows() const;
        void setCastShadows(bool value);
        const std::vector<uint32_t>& getMeshNodes();

    private:
        ResRef<MeshVertices> mesh;
        ResRef<PBRMaterial> material;
        bool castShadows;
        std::vector<uint32_t> meshNodes;
    };

}
