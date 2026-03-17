#include "CapsuleColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"
#include "Components/RigidBodyComponent.h"

namespace CgEngine {
    void CapsuleColliderComponentParams::verifyParams() const {}

    void CapsuleColliderComponent::onAttach(Scene& scene, CapsuleColliderComponentParams& params) {
        radius = params.radius;
        offset = params.offset;
        isTrigger = params.isTrigger;
        halfHeight = params.halfHeight;

        physicsMaterial = Application::get().getResourceManager().getResource<PhysicsMaterial>(params.material);
    }

    void CapsuleColliderComponent::onEnable(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity) && !isColliderAddedToActor) {
            scene.getComponent<RigidBodyComponent>(entity)->addCapsuleCollider(*physicsMaterial, radius, halfHeight, offset, isTrigger);
            isColliderAddedToActor = true;
        }
    }

    void CapsuleColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity)->removeCollider(PhysicsColliderType::Capsule);
        }
    }

    ResRef<PhysicsMaterial> CapsuleColliderComponent::getPhysicsMaterial() {
        return physicsMaterial;
    }

    float CapsuleColliderComponent::getRadius() const {
        return radius;
    }

    float CapsuleColliderComponent::getHalfHeight() const {
        return halfHeight;
    }

    glm::vec3 CapsuleColliderComponent::getOffset() const {
        return offset;
    }

    bool CapsuleColliderComponent::getIsTrigger() const {
        return isTrigger;
    }

    void CapsuleColliderComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CapsuleColliderComponent")) {
        }
    }
}
