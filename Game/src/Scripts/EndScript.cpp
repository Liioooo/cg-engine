#include "EndScript.h"
#include "CgEngine/Events/Input.h"
#include "CgEngine/Application.h"
#include "CgEngine/Components/UiCanvasComponent2D.h"

namespace Game {
    void EndScript::onEnable() {
        CgEngine::Input::setCursorMode(CgEngine::CursorMode::Normal);

        auto canvasEntity = findEntityById("canvas");
        auto canvas = canvasEntity.getComponent<CgEngine::UiCanvasComponent2D>();

        auto& againButton = *canvas->getCanvas()->getUIElement<CgEngine::UiRect>("againButton");
        addUiElementClickListener(againButton, [this]() {
            setActiveScene("scenes/game_scene.xml");
        });

        auto& exitButton = *canvas->getCanvas()->getUIElement<CgEngine::UiRect>("exitButton");
        addUiElementClickListener(exitButton, []() {
            CgEngine::Application::get().shutdown();
        });
    }
}
