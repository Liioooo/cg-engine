#include "Input.h"
#include "Application.h"
#include <GLFW/glfw3.h>

namespace CgEngine {
    bool Input::isKeyPressed(KeyCode keyCode) {
        auto& window = Application::get().getWindow().getWindowHandle();
        auto state = glfwGetKey(&window, static_cast<int>(keyCode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::isMouseButtonPressed(MouseButton mouseButton) {
        auto& window = Application::get().getWindow().getWindowHandle();
        auto state = glfwGetMouseButton(&window, static_cast<int>(mouseButton));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::getMousePosition() {
        auto& window = Application::get().getWindow().getWindowHandle();

        double x, y;
        glfwGetCursorPos(&window, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    float Input::getMouseX() {
        auto [x, y] = getMousePosition();
        return x;
    }

    float Input::getMouseY() {
        auto [x, y] = getMousePosition();
        return y;
    }

    void Input::setCursorMode(CursorMode cursorMode) {
        auto& window = Application::get().getWindow().getWindowHandle();
        glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<uint8_t>(cursorMode));
    }

    CursorMode Input::getCursorMode() {
        auto& window = Application::get().getWindow().getWindowHandle();
        return static_cast<CursorMode>(glfwGetInputMode(&window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
    }
}
