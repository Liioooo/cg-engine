#pragma once

#include "Resources/MeshVertices.h"
#include "Rendering/PBRMaterial.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct CustomShaderRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        std::string material;
        Material* customMaterial;
        bool castShadows = true;
        std::vector<std::string> meshNodes;
        uint32_t instanceCount;
        std::string shader;

        // some custom mesh

        void verifyParams() const;
    };

    class CustomShaderRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, CustomShaderRendererComponentParams& params);

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
