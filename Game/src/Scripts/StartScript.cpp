#include "StartScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Application.h"
#include "CgEngine/Components/UiCanvasComponent2D.h"

namespace Game {
    void StartScript::onEnable() {
        CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);

        auto canvasEntity = findEntityById("canvas");
        auto canvas = canvasEntity.getComponent<CgEngine::UiCanvasComponent2D>();

        auto& playButton = *canvas->getCanvas()->getUIElement<CgEngine::UiRect>("playButton");
        addUiElementClickListener(playButton, [this]() {
            setActiveScene("scenes/game_scene.xml");
        });

        auto& exitButton = *canvas->getCanvas()->getUIElement<CgEngine::UiRect>("exitButton");
        addUiElementClickListener(exitButton, []() {
            CgEngine::Application::get().shutdown();
        });

        auto& controlsButton = *canvas->getCanvas()->getUIElement<CgEngine::UiRect>("controlsButton");
        addUiElementClickListener(controlsButton, [this, canvasEntity]() {
            auto overlayEntity = instantiatePrefab("startMenuControlsOverlay");
            auto overlayCanvas = overlayEntity.getComponent<CgEngine::UiCanvasComponent2D>();

            canvasEntity.getComponent<CgEngine::UiCanvasComponent2D>()->setReceiveInputEvents(false);

            auto& overlayCloseButton = *overlayCanvas->getCanvas()->getUIElement<CgEngine::UiRect>("controlsCloseRect");
            addUiElementClickListener(overlayCloseButton, [canvasEntity, overlayEntity]() mutable {
                canvasEntity.getComponent<CgEngine::UiCanvasComponent2D>()->setReceiveInputEvents(true);
                overlayEntity.destroy();
            });
        });

    }
}
