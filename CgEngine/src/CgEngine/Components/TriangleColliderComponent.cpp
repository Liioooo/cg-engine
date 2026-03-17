#include "TriangleColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"
#include "Components/RigidBodyComponent.h"

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
    }

    void TriangleColliderComponent::onEnable(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)&& !isColliderAddedToActor) {
            scene.getComponent<RigidBodyComponent>(entity)->addTriangleCollider(*physicsMaterial, getPhysicsMesh(), isTrigger);
            isColliderAddedToActor = true;
        }
    }

    void TriangleColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity)->removeCollider(PhysicsColliderType::TriangleMesh);
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

    void TriangleColliderComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("TriangleColliderComponent")) {
        }
    }
}
