#include "Application.h"
#include "Logging.h"
#include "FileSystem.h"
#include "ImGui/ImGuiContext.h"
#include "imgui.h"
#include "ImGui/ImGuiWidgets.h"
#include "ImGui/ImGuiSceneView.h"
#include "OpenGLTimer.h"
#include "CgEngineSharedUtils/LoaderUtils.h"
#include "CgEngineSharedUtils/StringUtils.h"
#include "Rendering/GraphicsObjectsFactory.h"

namespace CgEngine {
    Application::Application(const std::string &settingsIni) : iniReader(settingsIni) {
        Logging::init();
        CG_LOGGING_INFO("Starting Application");
        instance = this;
    }

    Application::~Application() {
        CG_LOGGING_INFO("Shutting Down!");

        delete sceneRenderer;

        Renderer::shutdown();
        delete sceneManager;
        delete window;
    }

    Application &Application::get() {
        return *instance;
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
        applicationOptions.graphicsApi = iniReader.GetBoolean("application", "use_vulkan", true) ? GraphicsAPI::Vulkan : GraphicsAPI::OpenGL;

        std::vector<std::string> defaultLodDistances = LoaderUtils::getListFromString(iniReader.Get("application", "lod_distances", "10, 20, 100, 200, 400"));
        for (const auto& lodDistance: defaultLodDistances) {
            applicationOptions.defaultLodDistances.emplace_back(StringUtils::toFloat(lodDistance).value_or(0.0f));
        }

        GraphicsObjectsFactory::setGraphicsAPI(applicationOptions.graphicsApi);

        WindowSpecification windowSpecification;
        windowSpecification.width = iniReader.GetInteger("window", "width", 1280);
        windowSpecification.height = iniReader.GetInteger("window", "height", 720);
        windowSpecification.title = iniReader.Get("window", "title", "CG Engine");
        auto icon = std::filesystem::path(iniReader.Get("window", "icon", ""));
        windowSpecification.icon = icon.empty() ? icon : FileSystem::getAsGamePath(icon);
        windowSpecification.fullScreen = iniReader.GetBoolean("window", "fullscreen", false);
        windowSpecification.refreshRate = iniReader.GetInteger("window", "refresh_rate", 60);
        windowSpecification.vSync = iniReader.GetBoolean("window", "v_sync", true);
        windowSpecification.graphicsApi = applicationOptions.graphicsApi;

        window = new Window(windowSpecification, EVENT_BIND_FN(onEvent));
        Renderer::init(*window);

        sceneRenderer = new SceneRenderer(window->getWidth(), window->getHeight());

        sceneManager = new SceneManager();
        sceneManager->setViewportSize(window->getWidth(), window->getHeight());
        sceneManager->setActiveScene(iniReader.Get("game", "startScene", "default_scene.xml"));
    }

    void Application::run() {
        lastFrameTime = getTime();

        while (isRunning) {
            window->pollEvents();
            Renderer::beginFrame(*window);
            Renderer::beginImGuiFrame();

            Scene* activeScene = sceneManager->getActiveScene();
            activeScene->onUpdate(timeStep);
            activeScene->onRender(*sceneRenderer);

            CG_GPU_TIME_WRITE_RESULTS()

            renderImGuiWindow();
            Renderer::renderImGuiFrame();
            Renderer::endFrame(*window);

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

    SceneRenderer& Application::getSceneRenderer() {
        return *sceneRenderer;
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
        Renderer::setFramebufferResized();
    }

    void Application::onKeyPressed(KeyPressedEvent& event) {
        switch (event.getKeyCode()) {
            #ifdef CG_ENABLE_DEBUG_FEATURES
            case KeyCode::F1: {
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
                        ImGuiSceneView::renderSceneView(*sceneManager->getActiveScene());
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Application")) {
                        ImGuiWidgets::applicationOptions(applicationOptions);
                        ImGui::Separator();
                        ImGuiWidgets::performanceStats(timeStep.getSeconds(), sceneRenderer->getRenderingStats());
                        ImGui::EndTabItem();
                    }

                    ImGui::EndTabBar();
                }


                ImGui::End();
            }
        #endif
    }
}
