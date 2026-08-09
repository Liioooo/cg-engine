#include "SceneManager.h"
#include "SceneLoader.h"
#include "FileSystem.h"
#include "Application.h"

namespace CgEngine {

    SceneManager::~SceneManager() {
        delete nextScene;
        delete activeScene;

        nextScene = nullptr;
        activeScene = nullptr;
    }

    Scene* SceneManager::getActiveScene() {
        return activeScene;
    }

    void SceneManager::setActiveScene(const std::string &name) {
        if (!activeScene) {
            activeScene = SceneLoader::loadScene(getXMLFileForScene(name), viewportWidth, viewportHeight);
        } else {
            nextScene = SceneLoader::loadScene(getXMLFileForScene(name), viewportWidth, viewportHeight);
            switchedScenes = true;
        }
    }

    void SceneManager::setViewportSize(uint32_t width, uint32_t height) {
        viewportWidth = width;
        viewportHeight = height;
        if (activeScene != nullptr) {
            activeScene->onViewportResize(width, height);
        }
        if (nextScene != nullptr) {
            nextScene->onViewportResize(width, height);
        }
    }

    bool SceneManager::shouldSwapScenes() const {
        return switchedScenes;
    }

    void SceneManager::swapScenes() {
        delete activeScene;
        activeScene = nextScene;
        nextScene = nullptr;
        switchedScenes = false;
    }

    void SceneManager::destroyAllScenes() {
        delete nextScene;
        delete activeScene;

        nextScene = nullptr;
        activeScene = nullptr;
    }

    XMLFile& SceneManager::getXMLFileForScene(const string& name) {
        auto& xmlFile = xmlSceneFileCache[name];
        if (!xmlFile.isLoaded()) {
            xmlFile.load(FileSystem::getAsGamePath(name));
        }
        return xmlFile;
    }
}
