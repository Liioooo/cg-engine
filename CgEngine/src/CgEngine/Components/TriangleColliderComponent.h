#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Physics/PhysicsTriangleMesh.h"
#include "Resources/MeshVertices.h"

namespace CgEngine {

    struct TriangleColliderComponentParams {
        std::string assetFile;
        std::string meshNode;
        bool isTrigger;
        std::string material;
    };

    class TriangleColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, TriangleColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        PhysicsMaterial& getPhysicsMaterial();
        bool getIsTrigger() const;

        PhysicsTriangleMesh& getPhysicsMesh();

    private:
        MeshVertices* mesh;
        PhysicsMaterial* physicsMaterial;
        std::string meshNode;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
