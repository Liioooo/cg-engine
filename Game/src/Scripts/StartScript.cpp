#include "StartScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Application.h"

namespace Game {
    void StartScript::onAttach() {
        CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);
    }

    void StartScript::onMouseButtonPressed(CgEngine::MouseButtonPressedEvent& event) {
        auto& canvas = getComponent<CgEngine::UiCanvasComponent>(findEntityById("canvas"));

        if (canvas.hasUiElement("controlsOverlay")) {
            canvas.removeUIElement("controlsOverlay");
            canvas.removeUIElement("controlsImage");
            canvas.removeUIElement("controlsClose");
            return;
        }

        if (canvas.getUIElement<CgEngine::UiRect>("playButton")->isPointInside({event.getXPos(), event.getYPos()})) {
            setActiveScene("scenes/game_scene.xml");
        }

        if (!canvas.hasUiElement("controlsOverlay") && canvas.getUIElement<CgEngine::UiRect>("controlsButton")->isPointInside({event.getXPos(), event.getYPos()})) {
            auto* overlay = canvas.addUiRect("controlsOverlay");
            overlay->setWidth(0.8f, CgEngine::UIPosUnit::VWPercent);
            overlay->setHeight(0.85f, CgEngine::UIPosUnit::VHPercent);
            overlay->setFillColor({1.0f, 1.0f, 1.0f, 0.8f});
            overlay->setLeft(0.5f, CgEngine::UIPosUnit::VWPercent);
            overlay->setTop(0.5f, CgEngine::UIPosUnit::VHPercent);
            overlay->setXAlignment(CgEngine::UIXAlignment::Center);
            overlay->setYAlignment(CgEngine::UIYAlignment::Center);
            overlay->setZIndex(10);

            auto* controls = canvas.addUiRect("controlsImage");
            controls->setWidth(0.65f, CgEngine::UIPosUnit::VWPercent);
            controls->setHeight(0.8f, CgEngine::UIPosUnit::VHPercent);
            controls->setTexture("controls.png");
            controls->setLeft(0.5f, CgEngine::UIPosUnit::VWPercent);
            controls->setTop(0.5f, CgEngine::UIPosUnit::VHPercent);
            controls->setXAlignment(CgEngine::UIXAlignment::Center);
            controls->setYAlignment(CgEngine::UIYAlignment::Center);
            controls->setZIndex(11);

            auto* close = canvas.addUiText("controlsClose");
            close->setLeft(0.865f, CgEngine::UIPosUnit::VWPercent);
            close->setTop(0.125f, CgEngine::UIPosUnit::VHPercent);
            close->setXAlignment(CgEngine::UIXAlignment::Center);
            close->setYAlignment(CgEngine::UIYAlignment::Center);
            close->setZIndex(11);
            close->setText("X");
            close->setSize(0.05f, CgEngine::UIPosUnit::VWPercent);
            close->setColor({0.0f, 0.0f, 0.0f, 1.0f});
            close->setFont("SpaceMono-Bold.ttf");
        }

        if (canvas.getUIElement<CgEngine::UiRect>("exitButton")->isPointInside({event.getXPos(), event.getYPos()})) {
            CgEngine::Application::get().shutdown();
        }

    }

    void StartScript::onMouseMoved(CgEngine::MouseMovedEvent& event) {
        auto& canvas = getComponent<CgEngine::UiCanvasComponent>(findEntityById("canvas"));

        auto* playButton = canvas.getUIElement<CgEngine::UiRect>("playButton");
        auto* playText = canvas.getUIElement<CgEngine::UiText>("playText");
        if (playButton->isPointInside({event.getXPos(), event.getYPos()})) {
            playText->setColor({0.8f, 0.0f, 0.0f, 1.0f});
            playButton->setLineWidth(5);
        } else {
            playText->setColor({0.0f, 0.0f, 0.0f, 1.0f});
            playButton->setLineWidth(0);
        }

        auto* controlsButton = canvas.getUIElement<CgEngine::UiRect>("controlsButton");
        auto* controlsText = canvas.getUIElement<CgEngine::UiText>("controlsText");
        if (controlsButton->isPointInside({event.getXPos(), event.getYPos()})) {
            controlsText->setColor({0.8f, 0.0f, 0.0f, 1.0f});
            controlsButton->setLineWidth(5);
        } else {
            controlsText->setColor({0.0f, 0.0f, 0.0f, 1.0f});
            controlsButton->setLineWidth(0);
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
