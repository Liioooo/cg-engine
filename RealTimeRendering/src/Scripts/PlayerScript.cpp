#include "PlayerScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Events/KeyCodes.h"
#include "glm/gtx/rotate_vector.hpp"
#include "CgEngine/Application.h"
#include "CgEngine/Components/AnimatedMeshRendererComponent.h"
#include "CgEngine/Components/CharacterControllerComponent.h"
#include "CgEngine/Components/TransformComponent.h"

namespace RTR {
    void PlayerScript::onEnable() {
        prevMousePos = CgEngine::Input::getMousePosition();
    }

    void PlayerScript::fixedUpdate(CgEngine::TimeStep ts) {
        CgEngine::EntityHandle cameraEntity = findEntityById("humanCam");
        if (!cameraEntity.getComponent<CgEngine::CameraComponent>()->isPrimary() || CgEngine::Input::getCursorMode() != CgEngine::CursorMode::Locked) {
            return;
        }

        auto humanMeshEntity = findEntityById("humanMesh");

        auto playerMeshTransform = humanMeshEntity.getComponent<CgEngine::TransformComponent>();
        playerMeshTransform->setLocalRotationVec({0.0f, yaw + glm::pi<float>(), 0.0f});

        glm::vec3 cameraDirection = glm::normalize(glm::quat({0.0f, yaw, 0.0f}) * glm::vec3(0, 0, -1));
        auto movement = glm::vec3(0.0f);

        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::W)) {
            movement += cameraDirection;
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::A)) {
            movement += glm::rotateY(cameraDirection, glm::radians(90.0f));
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::D)) {
            movement += glm::rotateY(cameraDirection, glm::radians(-90.0f));
        }

        auto comp = getOwingEntity().getComponent<CgEngine::CharacterControllerComponent>();

        bool back = false;

        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::S)) {
            movement = -cameraDirection * ts.getSeconds() * 6.0f;
            back = true;
        } else {
            movement = (glm::length(movement) == 0.0f ? movement : normalize(movement)) * ts.getSeconds() * 6.0f;
        }

        auto animComp = humanMeshEntity.getComponent<CgEngine::AnimatedMeshRendererComponent>();
        animComp->setAnimationPlaying(glm::length(movement) > 0.0f);
        animComp->setAnimationSpeed(back ? -1.0f : 1.0f);

        comp->move(movement);
    }

    void PlayerScript::lateUpdate(CgEngine::TimeStep ts) {
        CgEngine::EntityHandle cameraEntity = findEntityById("humanCam");
        if (!cameraEntity.getComponent<CgEngine::CameraComponent>()->isPrimary()  || CgEngine::Input::getCursorMode() != CgEngine::CursorMode::Locked) {
            return;
        }

        auto mousePos = CgEngine::Input::getMousePosition();

        float mouseDeltaX = (prevMousePos.first - mousePos.first) * 0.001f;
        float mouseDeltaY = (prevMousePos.second - mousePos.second) * 0.0015f;

        prevMousePos = mousePos;

        auto cameraTransform = cameraEntity.getComponent<CgEngine::TransformComponent>();

        pitch = glm::clamp(pitch + mouseDeltaY, glm::radians(-80.0f), glm::radians(10.0f));
        yaw = glm::fmod(yaw + mouseDeltaX, glm::two_pi<float>());

        glm::vec3 cameraDirection;

        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::V)) {
            cameraDirection = glm::normalize(glm::quat({pitch, yaw + glm::pi<float>(), 0}) * glm::vec3(0, 0, -1));
            cameraTransform->setYawPitchRoll(yaw + glm::pi<float>(), pitch, 0);

        } else {
            cameraDirection = glm::normalize(glm::quat({pitch, yaw, 0}) * glm::vec3(0, 0, -1));
            cameraTransform->setYawPitchRoll(yaw, pitch, 0);
        }
        cameraTransform->setLocalPosition((cameraDirection * -cameraDistance) + cameraOffset);
    }

    void PlayerScript::onMouseScrolled(CgEngine::MouseScrolledEvent& event) {
        if (event.getYOffsetPos() > 0) {
            cameraDistance -= 0.4f;
        } else {
            cameraDistance += 0.4f;
        }
        cameraDistance = glm::clamp(cameraDistance, 3.0f, 6.0f);
    }

}
