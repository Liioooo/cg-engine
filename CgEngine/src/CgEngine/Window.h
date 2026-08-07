#pragma once

#define GLFW_INCLUDE_VULKAN
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
        Window() = default;
        ~Window();

        void init(const WindowSpecification& spec, std::function<void(Event&)>&& eventCallback);

        bool isVsync() const;
        void pollEvents();
        int getWidth() const;
        int getHeight() const;
        int getFramebufferWidth() const;
        int getFramebufferHeight() const;
        GLFWwindow& getWindowHandle() const;
        glm::vec2 getContentScale() const;
        std::vector<const char*> getRequiredVulkanExtensions() const;

        void setClipboardText(const char* string);

    private:
        bool vsync = false;
        int windowWidth;
        int windowHeight;
        int framebufferWidth;
        int framebufferHeight;
        GLFWwindow* window = nullptr;
        std::function<void(Event&)> eventCallback;

        static void errorCallback(int error, const char* description);
    };

}
