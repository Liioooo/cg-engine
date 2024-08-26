#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsConvexMesh.h"
#include "Resources/MeshVertices.h"

namespace CgEngine {

    struct ConvexColliderComponentParams {
        std::string assetFile;
        std::string meshNode;
        bool isTrigger;
        std::string material;
    };

    class ConvexColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, ConvexColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        PhysicsMaterial& getPhysicsMaterial();
        bool getIsTrigger() const;

        PhysicsConvexMesh& getPhysicsMesh();

    private:
        MeshVertices* mesh;
        PhysicsMaterial* physicsMaterial;
        std::string meshNode;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
