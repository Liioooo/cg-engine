#pragma once

#include "KeyCodes.h"

namespace CgEngine {

    class Input {
    public:
        static bool isKeyPressed(KeyCode keyCode);
        static bool isMouseButtonPressed(MouseButton mouseButton);
        static std::pair<float, float> getMousePosition();
        static float getMouseX();
        static float getMouseY();

        static void setCursorMode(CursorMode cursorMode);
        static CursorMode getCursorMode();
    };

}
