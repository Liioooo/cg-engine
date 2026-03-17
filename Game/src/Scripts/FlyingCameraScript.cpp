#include "CgEngine/Components/TransformComponent.h"
#include "FlyingCameraScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Events/KeyCodes.h"

namespace Game {
    void FlyingCameraScript::update(CgEngine::TimeStep ts) {
        if (!getOwingEntity().getComponent<CgEngine::CameraComponent>()->isPrimary()) {
            return;
        }

        if (CgEngine::Input::isMouseButtonPressed(CgEngine::MouseButton::MouseButtonLeft) && CgEngine::Input::getCursorMode() == CgEngine::CursorMode::Normal) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Locked);
            prevMousePos = CgEngine::Input::getMousePosition();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::Escape)) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);
        }

        auto mousePos = CgEngine::Input::getMousePosition();

        float mouseDeltaX = (prevMousePos.first - mousePos.first) * 0.001f;
        float mouseDeltaY = (prevMousePos.second - mousePos.second) * 0.001f;

        prevMousePos = mousePos;

        auto comp = getOwingEntity().getComponent<CgEngine::TransformComponent>();
        glm::vec3 pos = comp->getLocalPosition();

        if (CgEngine::Input::getCursorMode() == CgEngine::CursorMode::Locked) {
            pitch = glm::clamp(pitch + mouseDeltaY, glm::radians(-80.0f), glm::radians(80.0f));
            yaw = glm::fmod(yaw + mouseDeltaX, glm::two_pi<float>());
        }

        glm::vec3 front = glm::normalize(glm::quat({pitch, yaw, 0}) * glm::vec3(0, 0, -1));

        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::W)) {
            pos += front * 10.0f * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::S)) {
            pos -= front * 10.0f * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::A)) {
            pos -= glm::normalize(glm::cross(front, glm::vec3(0, 1, 0))) * 10.0f * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::D)) {
            pos += glm::normalize(glm::cross(front, glm::vec3(0, 1, 0))) * 10.0f * ts.getSeconds();
        }

        comp->setLocalPosition(pos);
        comp->setYawPitchRoll(yaw, pitch, 0);
    }
}
