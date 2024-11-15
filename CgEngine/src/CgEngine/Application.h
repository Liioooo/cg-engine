#pragma once

#include <INIReader.h>
#include "Scripting/ScriptManager.h"
#include "Resources/ResourceManager.h"
#include "Audio/AudioSystem.h"
#include "Rendering/SceneRenderer.h"
#include "Window.h"
#include "Events/WindowCloseEvent.h"
#include "Events/WindowResizeEvent.h"
#include "Events/KeyPressedEvent.h"
#include "Scene/SceneManager.h"
#include "TimeStep.h"

namespace CgEngine {

    struct ApplicationOptions {
        bool debugShowPhysicsColliders;
        bool debugShowBoundingBoxes;
        bool debugShowNormals;
        bool debugRenderLines;

        float anisotropicFiltering;
        uint32_t shadowMapResolution;
        bool enableBloom;
        bool enableHBAO;
    };

    class Application {
    public:
        explicit Application(const std::string& settingsIni);
        ~Application();

        static Application& get();

        void init();
        void run();
        void shutdown();
        float getTime();
        ApplicationOptions& getApplicationOptions();
        ScriptManager& getScriptManager();
        PhysicsSystem& getPhysicsSystem();
        ResourceManager& getResourceManager();
        SceneManager& getSceneManager();
        Window& getWindow();

        template<typename S>
        void registerNativeScript(const std::string& name) {
            scriptManager.registerNativeScript<S>(name);
        }

    private:
        INIReader iniReader;
        ApplicationOptions applicationOptions;
        ScriptManager scriptManager;
        SceneManager* sceneManager;
        PhysicsSystem physicsSystem;
        AudioSystem audioSystem;
        ResourceManager resourceManager;
        SceneRenderer* sceneRenderer;
        Window* window;
        bool isRunning = true;
        float lastFrameTime = getTime();
        TimeStep timeStep{};

        bool showImGuiWindow = false;

        void onEvent(Event& event);
        void onWindowClose(WindowCloseEvent& event);
        void onWindowResize(WindowResizeEvent& event);
        void onKeyPressed(KeyPressedEvent& event);

        void renderImGuiWindow();

        static inline Application* instance;
    };

}

