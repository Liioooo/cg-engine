#include "TriangleColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"

namespace CgEngine {
    void TriangleColliderComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty(), "TriangleColliderComponentParams: 'assetFile' is required.")
        CG_ASSERT(!meshNode.empty(), "TriangleColliderComponentParams: 'meshNode' is required.")
    }

    void TriangleColliderComponent::onAttach(Scene& scene, TriangleColliderComponentParams& params) {
        isTrigger = params.isTrigger;
        meshNode = params.meshNode;

        auto& resourceManager = Application::get().getResourceManager();

        physicsMaterial = resourceManager.getResource<PhysicsMaterial>(params.material);
        mesh = resourceManager.getResource<MeshVertices>(params.assetFile);

        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            colliderUuid = scene.getComponent<RigidBodyComponent>(entity).addTriangleCollider(*physicsMaterial, getPhysicsMesh(), isTrigger);
        }
    }

    void TriangleColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity).removeCollider(colliderUuid);
        }
    }

    ResRef<PhysicsMaterial> TriangleColliderComponent::getPhysicsMaterial() {
        return physicsMaterial;
    }

    bool TriangleColliderComponent::getIsTrigger() const {
        return isTrigger;
    }

    PhysicsTriangleMesh& TriangleColliderComponent::getPhysicsMesh() {
        return mesh->getPhysicsTriangleMeshForNode(meshNode);
    }
}
