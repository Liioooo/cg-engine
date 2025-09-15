#pragma once

#include <GLFW/glfw3.h>

namespace CgEngine {

    class ImGuiContext {
    public:
        static bool wantCaptureMouse();
        static bool wantCaptureKeyboard();
    };

}
