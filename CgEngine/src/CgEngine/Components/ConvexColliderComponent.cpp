#include "ConvexColliderComponent.h"
#include "Scene/Scene.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void ConvexColliderComponent::onAttach(Scene& scene, ConvexColliderComponentParams& params) {
        isTrigger = params.isTrigger;
        meshNode = params.meshNode;

        auto& resourceManager = GlobalObjectManager::getInstance().getResourceManager();

        physicsMaterial = resourceManager.getResource<PhysicsMaterial>(params.material);
        mesh = resourceManager.getResource<MeshVertices>(params.assetFile);

        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            colliderUuid = scene.getComponent<RigidBodyComponent>(entity).addConvexCollider(*physicsMaterial, getPhysicsMesh(), isTrigger);
        }
    }

    void ConvexColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity).removeCollider(colliderUuid);
        }
    }

    PhysicsMaterial& ConvexColliderComponent::getPhysicsMaterial() {
        return *physicsMaterial;
    }

    bool ConvexColliderComponent::getIsTrigger() const {
        return isTrigger;
    }

    PhysicsConvexMesh& ConvexColliderComponent::getPhysicsMesh() {
        return mesh->getPhysicsConvexMeshForNode(meshNode);
    }
}
