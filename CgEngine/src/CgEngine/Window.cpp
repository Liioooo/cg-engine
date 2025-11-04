#include "Window.h"
#include <glad/glad.h>
#include "Rendering/Helpers.h"
#include "Logging.h"
#include "Events/KeyPressedEvent.h"
#include "Events/KeyReleasedEvent.h"
#include "Events/MouseButtonPressedEvent.h"
#include "Events/MouseButtonReleasedEvent.h"
#include "Events/MouseMovedEvent.h"
#include "Events/MouseScrolledEvent.h"
#include "Events/WindowCloseEvent.h"
#include "Events/WindowResizeEvent.h"
#include "Rendering/Renderer.h"
#include "FileSystem.h"

namespace CgEngine {
    Window::Window(const WindowSpecification& spec, std::function<void(Event&)>&& eventCallback) : eventCallback(std::move(eventCallback)) {
        if (!glfwInit()) {
            CG_LOGGING_ERROR("Failed to init GLFW");
        }

        glfwSetErrorCallback(&Window::errorCallback);

        if (spec.graphicsApi == GraphicsAPI::OpenGL) {
            #ifdef CG_ENABLE_DEBUG_FEATURES
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
            #endif

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
        } else if (spec.graphicsApi == GraphicsAPI::Vulkan) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        if (spec.fullScreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

            if (spec.graphicsApi == GraphicsAPI::OpenGL) {
                glfwWindowHint(GLFW_RED_BITS, vidMode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, vidMode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, vidMode->blueBits);
            }

            glfwWindowHint(GLFW_REFRESH_RATE, vidMode->refreshRate);

            window = glfwCreateWindow(vidMode->width, vidMode->height, spec.title.c_str(), monitor, nullptr);

            windowWidth = vidMode->width;
            windowHeight = vidMode->height;
        } else {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_REFRESH_RATE, spec.refreshRate);
            window = glfwCreateWindow(spec.width, spec.height, spec.title.c_str(), nullptr, nullptr);

            windowWidth = spec.width;
            windowHeight = spec.height;
        }

        if (!window) {
            CG_LOGGING_ERROR("Failed to create Window");
        }

        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        if (!spec.icon.empty() && FileSystem::checkFileExists(spec.icon)) {
            GLFWimage icon;
            auto iconData = Helpers::loadImageData(spec.icon);
            icon.pixels = std::get<0>(iconData);
            icon.width = std::get<1>(iconData);
            icon.height = std::get<2>(iconData);
            glfwSetWindowIcon(window, 1, &icon);
            Helpers::freeImageData(std::get<0>(iconData));
        }

        if (spec.graphicsApi == GraphicsAPI::OpenGL) {
            glfwMakeContextCurrent(window);
            gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
        }

        vsync = spec.vSync;

        glfwSetWindowUserPointer(window, this);

        glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
            WindowCloseEvent e;
            static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
        });

        glfwSetWindowSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));

            self->windowWidth = width;
            self->windowHeight = height;
            glfwGetFramebufferSize(w, &self->framebufferWidth, &self->framebufferHeight);

            WindowResizeEvent e(width, height);
            self->eventCallback(e);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            double x, y;

            switch (action) {
                case GLFW_PRESS: {
                    glfwGetCursorPos(w, &x, &y);
                    MouseButtonPressedEvent e(static_cast<MouseButton>(button), static_cast<float>(x), static_cast<float>(y));
                    static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
                    break;
                }
                case GLFW_RELEASE: {
                    glfwGetCursorPos(w, &x, &y);
                    MouseButtonReleasedEvent e(static_cast<MouseButton>(button), static_cast<float>(x), static_cast<float>(y));
                    static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
                    break;
                }
            }
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
            MouseMovedEvent e(static_cast<float>(x), static_cast<float>(y));
            static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
        });

        glfwSetScrollCallback(window, [](GLFWwindow* w, double offsetX, double offsetY) {
            MouseScrolledEvent e(static_cast<float>(offsetX), static_cast<float>(offsetY));
            static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
        });

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent e(static_cast<KeyCode>(key), static_cast<ModifierKey>(mods));
                    static_cast<Window *>(glfwGetWindowUserPointer(w))->eventCallback(e);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent e(static_cast<KeyCode>(key), static_cast<ModifierKey>(mods));
                    static_cast<Window *>(glfwGetWindowUserPointer(w))->eventCallback(e);
                    break;
                }
            }
        });
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Window::isVsync() const {
        return vsync;
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    int Window::getWidth() const {
        return windowWidth;
    }

    int Window::getHeight() const {
        return windowHeight;
    }

    int Window::getFramebufferWidth() const {
        return framebufferWidth;
    }

    int Window::getFramebufferHeight() const {
        return framebufferHeight;
    }

    GLFWwindow& Window::getWindowHandle() const {
        return *window;
    }

    glm::vec2 Window::getContentScale() const {
        glm::vec2 out;
        glfwGetWindowContentScale(window, &out.x, &out.y);
        return out;
    }

    std::vector<const char*> Window::getRequiredVulkanExtensions() const {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }

    void Window::setClipboardText(const char* string) {
        glfwSetClipboardString(window, string);
    }

    void Window::errorCallback(int error, const char *description) {
        CG_LOGGING_ERROR("GlFW ERROR {0}: {1}", error, description);
    }

}
