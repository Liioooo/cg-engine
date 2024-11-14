#include "CharacterControllerComponent.h"
#include "Scene/Scene.h"
#include "imgui.h"

namespace CgEngine {
    void CharacterControllerComponentParams::verifyParams() const {}

    void CharacterControllerComponent::onAttach(Scene& scene, CharacterControllerComponentParams& params) {
        physicsController = scene.getPhysicsScene().createController(scene, entity, params.hasGravity, params.stepOffset, params.stepDownOffset, params.slopeLimit);
    }

    void CharacterControllerComponent::onDetach(Scene& scene) {
        delete physicsController;
    }

    void CharacterControllerComponent::move(glm::vec3 dir) {
        physicsController->move(dir);
    }

    void CharacterControllerComponent::setPosition(glm::vec3 pos) {
        physicsController->setPosition(pos);
    }

    void CharacterControllerComponent::jump(float strength) {
        physicsController->jump(strength);
    }

    bool CharacterControllerComponent::isGrounded() {
        return physicsController->isGrounded();
    }

    void CharacterControllerComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CharacterControllerComponent")) {
        }
    }
}
