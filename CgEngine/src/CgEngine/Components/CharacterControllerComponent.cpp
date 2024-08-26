#include "CharacterControllerComponent.h"
#include "Scene/Scene.h"

namespace CgEngine {
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
}