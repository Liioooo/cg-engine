#include "EndScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Application.h"

namespace Game {
    void EndScript::onAttach() {
        CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);
    }

    void EndScript::onMouseButtonPressed(CgEngine::MouseButtonPressedEvent& event) {
        auto& canvas = getComponent<CgEngine::UiCanvasComponent>(findEntityById("canvas"));

        if (canvas.getUIElement<CgEngine::UiRect>("againButton")->isPointInside({event.getXPos(), event.getYPos()})) {
            setActiveScene("scenes/game_scene.xml");
        }

        if (canvas.getUIElement<CgEngine::UiRect>("exitButton")->isPointInside({event.getXPos(), event.getYPos()})) {
            CgEngine::Application::get().shutdown();
        }

    }

    void EndScript::onMouseMoved(CgEngine::MouseMovedEvent& event) {
        auto& canvas = getComponent<CgEngine::UiCanvasComponent>(findEntityById("canvas"));

        auto* againButton = canvas.getUIElement<CgEngine::UiRect>("againButton");
        auto* againText = canvas.getUIElement<CgEngine::UiText>("againText");
        if (againButton->isPointInside({event.getXPos(), event.getYPos()})) {
            againText->setColor({0.8f, 0.0f, 0.0f, 1.0f});
            againButton->setLineWidth(5);
        } else {
            againText->setColor({0.0f, 0.0f, 0.0f, 1.0f});
            againButton->setLineWidth(0);
        }

        auto* exitButton = canvas.getUIElement<CgEngine::UiRect>("exitButton");
        auto* exitText = canvas.getUIElement<CgEngine::UiText>("exitText");
        if (exitButton->isPointInside({event.getXPos(), event.getYPos()})) {
            exitText->setColor({0.8f, 0.0f, 0.0f, 1.0f});
            exitButton->setLineWidth(5);
        } else {
            exitText->setColor({0.0f, 0.0f, 0.0f, 1.0f});
            exitButton->setLineWidth(0);
        }
    }
}
