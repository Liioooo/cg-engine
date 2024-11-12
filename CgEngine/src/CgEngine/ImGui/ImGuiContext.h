#pragma once

#include <GLFW/glfw3.h>

namespace CgEngine {

    class ImGuiContext {
    public:
        static void init(GLFWwindow* glfwWindow, float scale);
        static void shutdown();

        static void newFrame();
        static void render();

        static bool wantCaptureMouse();
        static bool wantCaptureKeyboard();
    };

}
