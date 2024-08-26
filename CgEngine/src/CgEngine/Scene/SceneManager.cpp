#include "SceneManager.h"
#include "GlobalObjectManager.h"
#include "Resources/XMLFile.h"
#include "SceneLoader.h"
#include "FileSystem.h"

namespace CgEngine {

    SceneManager::~SceneManager() {
        if (nextScene != nullptr) {
            delete nextScene;
        }
        delete activeScene;
    }

    Scene* SceneManager::getActiveScene() {
        if (nextScene != nullptr) {
            delete activeScene;
            activeScene = nextScene;
            nextScene = nullptr;
        }
        return activeScene;
    }

    void SceneManager::setActiveScene(const std::string &name) {
        switchedScenes = true;
        auto* xmlSceneFile = GlobalObjectManager::getInstance().getResourceManager().getResource<XMLFile>(FileSystem::getAsGamePath(name));
        nextScene = SceneLoader::loadScene(xmlSceneFile, viewportWidth, viewportHeight);
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

    bool SceneManager::hadSceneSwitch() {
        bool out = switchedScenes;
        switchedScenes = false;
        return out;
    }
}
