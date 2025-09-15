#pragma once

#include <GLFW/glfw3.h>
#include "Events/Event.h"
#include "GraphicsApi.h"

namespace CgEngine {

    struct WindowSpecification {
        std::string title;
        std::filesystem::path icon;
        uint32_t width = 1280;
        uint32_t height = 720;
        uint32_t refreshRate = 60;
        bool fullScreen = false;
        bool vSync = true;
        GraphicsAPI graphicsApi;
    };

    class Window {
    public:
        Window(const WindowSpecification& spec, std::function<void(Event&)>&& eventCallback);
        ~Window();

        void setVsync(bool enabled);
        inline bool isVsync();
        void pollEvents();
        int getWidth();
        int getHeight();
        int getFramebufferWidth() const;
        int getFramebufferHeight() const;
        GLFWwindow& getWindowHandle() const;
        glm::vec2 getContentScale() const;

        void setClipboardText(const char* string);

    private:
        bool vsync = false;
        int windowWidth;
        int windowHeight;
        int framebufferWidth;
        int framebufferHeight;
        GLFWwindow* window;
        const std::function<void(Event&)> eventCallback;

        static void errorCallback(int error, const char* description);
    };

}
