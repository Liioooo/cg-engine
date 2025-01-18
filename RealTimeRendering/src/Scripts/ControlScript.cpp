#include "ControlScript.h"
#include "CgEngine/Events/KeyCodes.h"
#include "CgEngine/Events/Input.h"

namespace RTR {
    void ControlScript::update(CgEngine::TimeStep ts) {
        if (CgEngine::Input::isMouseButtonPressed(CgEngine::MouseButton::MouseButtonLeft) && CgEngine::Input::getCursorMode() == CgEngine::CursorMode::Normal) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Locked);
        }
        if (CgEngine::Input::isKeyPressed(CgEngine::KeyCode::Escape)) {
            CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);
        }
    }

    void ControlScript::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::F10) {
            flyCam = !flyCam;

            auto& flyCamera = getComponent<CgEngine::CameraComponent>(findEntityById("camera"));
            auto& humanCamera = getComponent<CgEngine::CameraComponent>(findEntityById("humanCam"));

            if (flyCam) {
                flyCamera.setPrimary(true);
                humanCamera.setPrimary(false);
            } else {
                flyCamera.setPrimary(false);
                humanCamera.setPrimary(true);
            }
        }
    }

}
