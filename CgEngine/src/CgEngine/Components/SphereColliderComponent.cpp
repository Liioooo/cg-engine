#include "SphereColliderComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"
#include "Components/RigidBodyComponent.h"

namespace CgEngine {
    void SphereColliderComponentParams::verifyParams() const {}

    void SphereColliderComponent::onAttach(Scene& scene, SphereColliderComponentParams& params) {
        radius = params.radius;
        offset = params.offset;
        isTrigger = params.isTrigger;

        physicsMaterial = Application::get().getResourceManager().getResource<PhysicsMaterial>(params.material);
    }

    void SphereColliderComponent::onEnable(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity) && !isColliderAddedToActor) {
            scene.getComponent<RigidBodyComponent>(entity)->addSphereCollider(*physicsMaterial, radius, offset, isTrigger);
            isColliderAddedToActor = true;
        }
    }

    void SphereColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity)->removeCollider(PhysicsColliderType::Sphere);
        }
    }

    ResRef<PhysicsMaterial> SphereColliderComponent::getPhysicsMaterial() {
        return physicsMaterial;
    }

    float SphereColliderComponent::getRadius() const {
        return radius;
    }

    glm::vec3 SphereColliderComponent::getOffset() const {
        return offset;
    }

    bool SphereColliderComponent::getIsTrigger() const {
        return isTrigger;
    }

    void SphereColliderComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("SphereColliderComponent")) {
        }
    }
}
