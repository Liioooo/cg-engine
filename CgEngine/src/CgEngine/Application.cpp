#include "Application.h"
#include "Logging.h"
#include "FileSystem.h"
#include "ImGui/ImGuiContext.h"
#include "imgui.h"
#include "ImGui/ImGuiWidgets.h"

namespace CgEngine {
    Application::Application(const std::string &settingsIni) : iniReader(settingsIni) {
        CgEngine::Logging::init();
        CG_LOGGING_INFO("Starting Application");
        Application::instance = this;
    }

    Application::~Application() {
        CG_LOGGING_INFO("Shutting Down!");

        delete sceneRenderer;

        Renderer::shutdown();
        delete sceneManager;
        delete window;
    }

    Application &Application::get() {
        return *Application::instance;
    }

    void Application::init() {
        applicationOptions.debugShowPhysicsColliders = iniReader.GetBoolean("application", "debug_show_physics_colliders", false);
        applicationOptions.debugShowBoundingBoxes = iniReader.GetBoolean("application", "debug_show_bounding_boxes", false);
        applicationOptions.debugShowNormals = iniReader.GetBoolean("application", "debug_show_normals", false);
        applicationOptions.debugRenderLines = iniReader.GetBoolean("application", "debug_render_lines", false);
        applicationOptions.anisotropicFiltering = static_cast<float>(iniReader.GetReal("application", "anisotropic_filtering", 1.0));
        applicationOptions.shadowMapResolution = iniReader.GetInteger("application", "shadow_map_resolution", 2048);
        applicationOptions.enableBloom = iniReader.GetBoolean("application", "enable_bloom", true);
        applicationOptions.enableHBAO = iniReader.GetBoolean("application", "enable_hbao", true);

        WindowSpecification windowSpecification;
        windowSpecification.width = iniReader.GetInteger("window", "width", 1280);
        windowSpecification.height = iniReader.GetInteger("window", "height", 720);
        windowSpecification.title = iniReader.Get("window", "title", "CG Engine");
        std::string icon = iniReader.Get("window", "icon", "");
        windowSpecification.icon = icon.empty() ? icon : FileSystem::getAsGamePath(icon);
        windowSpecification.fullScreen = iniReader.GetBoolean("window", "fullscreen", false);
        windowSpecification.refreshRate = iniReader.GetInteger("window", "refresh_rate", 60);
        windowSpecification.vSync = iniReader.GetBoolean("window", "v_sync", true);

        window = new Window(windowSpecification, EVENT_BIND_FN(onEvent));

        sceneRenderer = new SceneRenderer(window->getWidth(), window->getHeight());

        sceneManager = new SceneManager();
        sceneManager->setViewportSize(window->getWidth(), window->getHeight());
        sceneManager->setActiveScene(iniReader.Get("game", "startScene", "default_scene.xml"));
    }

    void Application::run() {
        lastFrameTime = getTime();

        while (isRunning) {
            window->pollEvents();
            ImGuiContext::newFrame();

            Scene* activeScene = sceneManager->getActiveScene();
            activeScene->onUpdate(timeStep);
            activeScene->onRender(*sceneRenderer);

            renderImGuiWindow();
            ImGuiContext::render();

            window->swapBuffers();

            float time = getTime();
            timeStep = time - lastFrameTime;
            lastFrameTime = time;

            if (sceneManager->shouldSwapScenes()) {
                sceneManager->swapScenes();
                resourceManager.unloadUnusedResources();
                timeStep = 0.0f;
            }
        }
    }

    void Application::shutdown() {
        isRunning = false;
    }

    float Application::getTime() {
        return static_cast<float>(glfwGetTime());
    }

    ApplicationOptions& Application::getApplicationOptions() {
        return applicationOptions;
    }

    ScriptManager& Application::getScriptManager() {
        return scriptManager;
    }

    PhysicsSystem& Application::getPhysicsSystem() {
        return physicsSystem;
    }

    ResourceManager& Application::getResourceManager() {
        return resourceManager;
    }

    SceneManager& Application::getSceneManager() {
        return *sceneManager;
    }

    Window &Application::getWindow() {
        return *window;
    }

    void Application::onEvent(Event& event) {
        EventDispatcher eventDispatcher(event);

        eventDispatcher.dispatch<WindowCloseEvent>(EVENT_BIND_FN(onWindowClose));
        eventDispatcher.dispatch<WindowResizeEvent>(EVENT_BIND_FN(onWindowResize));

        if (ImGuiContext::wantCaptureKeyboard() && event.isKeyboardEvent()) {
            return;
        }
        if (ImGuiContext::wantCaptureMouse() && event.isMouseEvent()) {
            return;
        }

        eventDispatcher.dispatch<KeyPressedEvent>(EVENT_BIND_FN(onKeyPressed));

        if (!event.wasHandled()) {
            sceneManager->getActiveScene()->onEvent(event);
        }
    }

    void Application::onWindowClose(WindowCloseEvent &event) {
        shutdown();
        event.stopPropagation();
    }

    void Application::onWindowResize(WindowResizeEvent &event) {
        sceneManager->setViewportSize(event.getWidth(), event.getHeight());
        sceneRenderer->setViewportSize(event.getWidth(), event.getHeight());
    }

    void Application::onKeyPressed(KeyPressedEvent& event) {
        switch (event.getKeyCode()) {
            #ifdef CG_ENABLE_DEBUG_FEATURES
            case KeyCode::F11: {
                showImGuiWindow = !showImGuiWindow;
                event.stopPropagation();
                break;
            }
            #endif
        }
    }

    void Application::renderImGuiWindow() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            if (showImGuiWindow) {
                ImGui::Begin("Debug Info", &showImGuiWindow);

                if (ImGui::BeginTabBar("#main-tabbar")) {
                    if (ImGui::BeginTabItem("Scene")) {
                        ImGui::Text("scene");

                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Application")) {
                        ImGuiWidgets::applicationOptions(applicationOptions);
                        ImGui::Separator();
                        ImGuiWidgets::performanceStats(timeStep.getSeconds(), sceneRenderer->getRenderingStats());
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Shaders")) {
                        ImGuiWidgets::shaders(sceneRenderer->getShaderMap(), resourceManager);
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }


                ImGui::End();
            }
        #endif
    }
}
