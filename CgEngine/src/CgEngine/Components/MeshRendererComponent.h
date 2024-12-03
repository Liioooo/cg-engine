#pragma once

#include "Resources/Mesh.h"
#include "Resources/CustomMesh.h"
#include "Rendering/PBRMaterial.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct MeshRendererComponentParams {
        std::string assetFile;
        std::string mesh;
        CustomMesh* customMesh = nullptr;
        std::string material;
        bool castShadows = true;
        bool enableCulling = true;
        std::vector<std::string> meshNodes;

        void verifyParams() const;
    };

    class MeshRendererComponent : public Component {
    public:
        using Component::Component;
        using Params = MeshRendererComponentParams;

        void onAttach(Scene& scene, MeshRendererComponentParams& params);
        void onRenderImGui() override;

        ResRef<MeshVertices> getMeshVertices();
        CustomMesh* getCustomMesh();
        Mesh* getRenderMesh();
        ResRef<PBRMaterial> getMaterial();
        bool getCastShadows() const;
        void setCastShadows(bool value);
        bool getCullingEnabled() const;
        void setCullingEnabled(bool value);
        const std::vector<uint32_t>& getMeshNodes();

        bool isActive() const;
        void setActive(bool a);

        void setCustomMesh(CustomMesh* mesh);

    private:
        ResRef<MeshVertices> mesh;
        CustomMesh* customMesh = nullptr;
        ResRef<PBRMaterial> material;
        bool castShadows;
        bool enableCulling;
        std::vector<uint32_t> meshNodes;
        bool active = true;
    };

}
