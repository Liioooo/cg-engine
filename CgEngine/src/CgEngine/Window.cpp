#include "Window.h"
#include <glad/glad.h>
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

namespace CgEngine {
    Window::Window(const WindowSpecification& spec, std::function<void(Event&)>&& eventCallback) : eventCallback(std::move(eventCallback)) {
        if (!glfwInit()) {
            CG_LOGGING_ERROR("Failed to init GLFW");
        }

        glfwSetErrorCallback(&Window::errorCallback);

#ifdef CG_ENABLE_DEBUG_FEATURES
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


        if (spec.fullScreen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, vidMode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, vidMode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, vidMode->blueBits);
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

        if (!spec.icon.empty()) {
            GLFWimage icon;
            auto iconData = TextureUtils::loadImageData(spec.icon);
            icon.pixels = std::get<0>(iconData);
            icon.width = std::get<1>(iconData);
            icon.height = std::get<2>(iconData);
            glfwSetWindowIcon(window, 1, &icon);
            TextureUtils::freImageData(std::get<0>(iconData));
        }

        glfwMakeContextCurrent(window);
        setVsync(spec.vSync);

        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

#ifdef CG_ENABLE_DEBUG_FEATURES
            glDebugMessageCallback(&Window::debugCallback, nullptr);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

        Renderer::init();

        glfwSetWindowUserPointer(window, this);

        glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
            WindowCloseEvent e;
            static_cast<Window*>(glfwGetWindowUserPointer(w))->eventCallback(e);
        });

        glfwSetWindowSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));

            self->windowWidth = width;
            self->windowHeight = height;

            WindowResizeEvent e(width, height);
            self->eventCallback(e);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            double x, y;

            switch (action) {
                case GLFW_PRESS: {
                    glfwGetCursorPos(w, &x, &y);
                    MouseButtonPressedEvent e(static_cast<MouseButton>(button), static_cast<float>(x), static_cast<float>(y));
                    static_cast<Window *>(glfwGetWindowUserPointer(w))->eventCallback(e);
                    break;
                }
                case GLFW_RELEASE: {
                    glfwGetCursorPos(w, &x, &y);
                    MouseButtonReleasedEvent e(static_cast<MouseButton>(button), static_cast<float>(x), static_cast<float>(y));
                    static_cast<Window *>(glfwGetWindowUserPointer(w))->eventCallback(e);
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

    void Window::setVsync(bool enabled) {
        vsync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    bool Window::isVsync() {
        return vsync;
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::swapBuffers() {
        glfwSwapBuffers(window);
    }

    uint32_t Window::getWidth() {
        return windowWidth;
    }

    uint32_t Window::getHeight() {
        return windowHeight;
    }

    GLFWwindow &Window::getWindowHandle() {
        return *window;
    }

    void Window::errorCallback(int error, const char *description) {
        CG_LOGGING_ERROR("GlFW ERROR {0}: {1}", error, description);
    }

    void Window::debugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *message, const void* userParam) {
        if (id == 131185 || id == 131218) {
            return; // ignore performance warnings (buffer uses GPU memory, shader recompilation) from nvidia
        }

        std::stringstream stringStream;
        std::string sourceString;
        std::string typeString;
        std::string severityString;

        switch (source) {
            case GL_DEBUG_SOURCE_API: {
                sourceString = "API";
                break;
            }
            case GL_DEBUG_SOURCE_APPLICATION: {
                sourceString = "Application";
                break;
            }
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
                sourceString = "Window System";
                break;
            }
            case GL_DEBUG_SOURCE_SHADER_COMPILER: {
                sourceString = "Shader Compiler";
                break;
            }
            case GL_DEBUG_SOURCE_THIRD_PARTY: {
                sourceString = "Third Party";
                break;
            }
            case GL_DEBUG_SOURCE_OTHER: {
                sourceString = "Other";
                break;
            }
            default: {
                sourceString = "Unknown";
                break;
            }
        }

        switch (type) {
            case GL_DEBUG_TYPE_ERROR: {
                typeString = "Error";
                break;
            }
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
                typeString = "Deprecated Behavior";
                break;
            }
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
                typeString = "Undefined Behavior";
                break;
            }
            case GL_DEBUG_TYPE_PERFORMANCE: {
                typeString = "Performance";
                break;
            }
            case GL_DEBUG_TYPE_OTHER: {
                typeString = "Other";
                break;
            }
            default: {
                typeString = "Unknown";
                break;
            }
        }

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: {
                severityString = "High";
                break;
            }
            case GL_DEBUG_SEVERITY_MEDIUM: {
                severityString = "Medium";
                break;
            }
            case GL_DEBUG_SEVERITY_LOW: {
                severityString = "Low";
                break;
            }
            default: {
                severityString = "Unknown";
                break;
            }
        }

        stringStream << "OpenGL Error: " << message;
        stringStream << " [Source = " << sourceString;
        stringStream << ", Type = " << typeString;
        stringStream << ", Severity = " << severityString;
        stringStream << ", ID = " << id << "]";

        CG_LOGGING_WARNING(stringStream.str());
    }

}
