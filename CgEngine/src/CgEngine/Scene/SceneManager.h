#pragma once

#include "Scene.h"

namespace CgEngine {

    class SceneManager {
    public:
        SceneManager() = default;
        ~SceneManager();

        Scene* getActiveScene();
        void setActiveScene(const std::string& name);
        void setViewportSize(uint32_t width, uint32_t height);

        bool hadSceneSwitch();

    private:
        Scene* activeScene = nullptr;
        Scene* nextScene = nullptr;

        uint32_t viewportWidth;
        uint32_t viewportHeight;

        bool switchedScenes = false;
    };

}



