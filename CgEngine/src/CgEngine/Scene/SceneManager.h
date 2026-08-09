#pragma once

#include "Scene.h"
#include "XMLFile.h"

namespace CgEngine {

    class SceneManager {
    public:
        SceneManager() = default;
        ~SceneManager();

        Scene* getActiveScene();
        void setActiveScene(const std::string& name);
        void setViewportSize(uint32_t width, uint32_t height);

        bool shouldSwapScenes() const;
        void swapScenes();

        void destroyAllScenes();

    private:
        XMLFile& getXMLFileForScene(const std::string& name);

        Scene* activeScene = nullptr;
        Scene* nextScene = nullptr;

        uint32_t viewportWidth;
        uint32_t viewportHeight;

        bool switchedScenes = false;

        std::unordered_map<std::string, XMLFile> xmlSceneFileCache;
    };

}



