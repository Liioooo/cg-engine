#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsConvexMesh.h"
#include "Resources/MeshVertices.h"

namespace CgEngine {

    struct ConvexColliderComponentParams {
        std::string assetFile;
        std::string meshNode;
        bool isTrigger = false;
        std::string material = "default-physics-material";

        void verifyParams() const;
    };

    class ConvexColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;
        using Params = ConvexColliderComponentParams;

        void onAttach(Scene& scene, ConvexColliderComponentParams& params);
        void onEnable(Scene& scene) override;
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        ResRef<PhysicsMaterial> getPhysicsMaterial();
        bool getIsTrigger() const;

        PhysicsConvexMesh& getPhysicsMesh();

    private:
        ResRef<MeshVertices> mesh;
        ResRef<PhysicsMaterial> physicsMaterial;
        std::string meshNode;
        bool isTrigger;
        bool isColliderAddedToActor = false;
    };

}
