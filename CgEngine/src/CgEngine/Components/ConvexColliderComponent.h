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

        void onAttach(Scene& scene, ConvexColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        ResRef<PhysicsMaterial> getPhysicsMaterial();
        bool getIsTrigger() const;

        PhysicsConvexMesh& getPhysicsMesh();

    private:
        ResRef<MeshVertices> mesh;
        ResRef<PhysicsMaterial> physicsMaterial;
        std::string meshNode;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
