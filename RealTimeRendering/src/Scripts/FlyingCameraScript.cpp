#include "FlyingCameraScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Events/KeyCodes.h"

namespace RTR {
    void FlyingCameraScript::update(CgEngine::TimeStep ts) {
        if (!getComponent<CgEngine::CameraComponent>().isPrimary()) {
            return;
        }

        if (CgEngine::Input::isMouseButtonPressed(CgEngine::MouseButton::MouseButtonLeft) && CgEngine::Input::getCursorMode() == CgEngine::CursorMode::Normal) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Locked);
            prevMousePos = CgEngine::Input::getMousePosition();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::Escape)) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);
        }

        if (!manualControl) {
            return;
        }

        auto mousePos = CgEngine::Input::getMousePosition();

        float mouseDeltaX = (prevMousePos.first - mousePos.first) * 0.001f;
        float mouseDeltaY = (prevMousePos.second - mousePos.second) * 0.001f;

        prevMousePos = mousePos;

        auto& comp = getComponent<CgEngine::TransformComponent>();
        glm::vec3 pos = comp.getLocalPosition();

        if (CgEngine::Input::getCursorMode() == CgEngine::CursorMode::Locked) {
            pitch = glm::clamp(pitch + mouseDeltaY, glm::radians(-80.0f), glm::radians(80.0f));
            yaw = glm::fmod(yaw + mouseDeltaX, glm::two_pi<float>());
        }

        glm::vec3 front = glm::normalize(glm::quat({pitch, yaw, 0}) * glm::vec3(0, 0, -1));
        float factor = getParameterMap().getAsFloat("normal").value_or(10.0f);
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::LeftShift)) {
            factor = getParameterMap().getAsFloat("fast").value_or(30.0f);
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::LeftControl)) {
            factor = getParameterMap().getAsFloat("slow").value_or(5.0f);
        }

        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::W)) {
            pos += front * factor * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::S)) {
            pos -= front * factor * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::A)) {
            pos -= glm::normalize(glm::cross(front, glm::vec3(0, 1, 0))) * factor * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::D)) {
            pos += glm::normalize(glm::cross(front, glm::vec3(0, 1, 0))) * factor * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::Q)) {
            pos += glm::normalize(glm::vec3(0, -1, 0)) * factor * ts.getSeconds();
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::E)) {
            pos += glm::normalize(glm::vec3(0, 1, 0)) * factor * ts.getSeconds();
        }

        comp.setLocalPosition(pos);
        comp.setYawPitchRoll(yaw, pitch, 0);
    }

    void FlyingCameraScript::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::F12) {
            getComponent<CgEngine::AnimationComponent>().setAnimationPlaying(manualControl);
            manualControl = !manualControl;
            if (!manualControl) {
                return;
            }
            auto rot = getComponent<CgEngine::TransformComponent>().getLocalRotationVec();
            yaw = rot.y;
            pitch = rot.x;
            if (glm::abs(glm::fmod(rot.z + glm::two_pi<float>(), glm::pi<float>()) - glm::pi<float>()) < 0.001) {
                // glm::eulerAngles normalises yaw between -90 and 90, if this would not be possible, it compensates by
                // using large values for pitch and yaw, i.e. setting them to 180 to flip everything, in our case we
                // only want a roll of 0 anyway
                // https://gamedev.stackexchange.com/questions/183771/euler-angle-and-quaternion-conversion-become-weird-when-yaw-is-bigger-than-90-de
                if (pitch < 0) {
                    pitch = glm::fmod(pitch + glm::two_pi<float>(), glm::pi<float>());
                    yaw = -(yaw + glm::pi<float>());
                } else {
                    pitch = glm::fmod(pitch - glm::two_pi<float>(), glm::pi<float>());
                    yaw = -(yaw - glm::pi<float>());
                }
            }
            prevMousePos = CgEngine::Input::getMousePosition();
        }
    }

}
