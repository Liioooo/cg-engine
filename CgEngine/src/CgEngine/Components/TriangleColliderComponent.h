#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsTriangleMesh.h"
#include "Resources/MeshVertices.h"

namespace CgEngine {

    struct TriangleColliderComponentParams {
        std::string assetFile;
        std::string meshNode;
        bool isTrigger = false;
        std::string material = "default-physics-material";

        void verifyParams() const;
    };

    class TriangleColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, TriangleColliderComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        ResRef<PhysicsMaterial> getPhysicsMaterial();
        bool getIsTrigger() const;

        PhysicsTriangleMesh& getPhysicsMesh();

    private:
        ResRef<MeshVertices> mesh;
        ResRef<PhysicsMaterial> physicsMaterial;
        std::string meshNode;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
