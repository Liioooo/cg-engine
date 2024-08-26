#pragma once

#include "Resources/MeshVertices.h"
#include "Rendering/Material.h"
#include "Component.h"

namespace CgEngine {

    struct MeshRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        std::string material;
        bool castShadows;
        std::vector<std::string> meshNodes;
    };

    class MeshRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, MeshRendererComponentParams& params);

        MeshVertices& getMeshVertices();
        Material* getMaterial();
        bool getCastShadows() const;
        void setCastShadows(bool value);
        const std::vector<uint32_t>& getMeshNodes();

    private:
        MeshVertices* mesh;
        Material* material;
        bool castShadows;
        std::vector<uint32_t> meshNodes;
    };

}
