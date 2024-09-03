#include "ConvexColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"

namespace CgEngine {
    void ConvexColliderComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty(), "ConvexColliderComponentParams: 'assetFile' is required.")
        CG_ASSERT(!meshNode.empty(), "ConvexColliderComponentParams: 'meshNode' is required.")
    }

    void ConvexColliderComponent::onAttach(Scene& scene, ConvexColliderComponentParams& params) {
        isTrigger = params.isTrigger;
        meshNode = params.meshNode;

        auto& resourceManager = Application::get().getResourceManager();

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
