#include "CharacterControllerComponent.h"
#include "Scene/Scene.h"
#include "imgui.h"

namespace CgEngine {
    void CharacterControllerComponentParams::verifyParams() const {}

    void CharacterControllerComponent::onAttach(Scene& scene, CharacterControllerComponentParams& params) {
       componentParams = params;
    }

    void CharacterControllerComponent::onEnable(Scene& scene) {
        physicsController = scene.getPhysicsScene().createController(scene, entity, componentParams.hasGravity, componentParams.stepOffset, componentParams.stepDownOffset, componentParams.slopeLimit);
    }

    void CharacterControllerComponent::onDetach(Scene& scene) {
        delete physicsController;
    }

    void CharacterControllerComponent::move(glm::vec3 dir) {
        CG_ASSERT(physicsController != nullptr, "PhysicsController is null in CharacterControllerComponent!")
        physicsController->move(dir);
    }

    void CharacterControllerComponent::setPosition(glm::vec3 pos) {
        CG_ASSERT(physicsController != nullptr, "PhysicsController is null in CharacterControllerComponent!")
        physicsController->setPosition(pos);
    }

    void CharacterControllerComponent::jump(float strength) {
        CG_ASSERT(physicsController != nullptr, "PhysicsController is null in CharacterControllerComponent!")
        physicsController->jump(strength);
    }

    bool CharacterControllerComponent::isGrounded() {
        CG_ASSERT(physicsController != nullptr, "PhysicsController is null in CharacterControllerComponent!")
        return physicsController->isGrounded();
    }

    void CharacterControllerComponent::setGravityEnabled(bool enabled) {
        CG_ASSERT(physicsController != nullptr, "PhysicsController is null in CharacterControllerComponent!")
        physicsController->setHasGravity(enabled);
    }

    void CharacterControllerComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CharacterControllerComponent")) {
        }
    }
}
