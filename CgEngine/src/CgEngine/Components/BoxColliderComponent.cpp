#include "BoxColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"
#include "Components/RigidBodyComponent.h"

namespace CgEngine {
    void BoxColliderComponentParams::verifyParams() const {}

    void BoxColliderComponent::onAttach(Scene& scene, BoxColliderComponentParams& params) {
        halfSize = params.halfSize;
        offset = params.offset;
        isTrigger = params.isTrigger;

        physicsMaterial = Application::get().getResourceManager().getResource<PhysicsMaterial>(params.material);
    }

    void BoxColliderComponent::onEnable(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity) && !isColliderAddedToActor) {
            scene.getComponent<RigidBodyComponent>(entity)->addBoxCollider(*physicsMaterial, halfSize, offset, isTrigger);
            isColliderAddedToActor = true;
        }
    }

    void BoxColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity)->removeCollider(PhysicsColliderType::Box);
        }
    }

    ResRef<PhysicsMaterial> BoxColliderComponent::getPhysicsMaterial() {
        return physicsMaterial;
    }

    glm::vec3 BoxColliderComponent::getHalfSize() const {
        return halfSize;
    }

    glm::vec3 BoxColliderComponent::getOffset() const {
        return offset;
    }

    bool BoxColliderComponent::getIsTrigger() const {
        return isTrigger;
    }

    void BoxColliderComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("BoxColliderComponent")) {
        }
    }
}
