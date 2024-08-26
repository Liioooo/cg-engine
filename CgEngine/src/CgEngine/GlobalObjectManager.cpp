#pragma once

#include "GlobalObjectManager.h"

namespace CgEngine {

    GlobalObjectManager &GlobalObjectManager::getInstance() {
        static GlobalObjectManager instance;
        return instance;
    }

    CgEngine::GlobalObjectManager::GlobalObjectManager() {
        sceneManager = new SceneManager();
        resourceManager = new ResourceManager();
        scriptManager = new ScriptManager();
        physicsSystem = new PhysicsSystem();
    }

    GlobalObjectManager::~GlobalObjectManager() {
        delete sceneManager;
        delete resourceManager;
        delete scriptManager;
        delete physicsSystem;
    }

    SceneManager& GlobalObjectManager::getSceneManager() const {
        return *sceneManager;
    }

    ResourceManager& GlobalObjectManager::getResourceManager() const {
        return *resourceManager;
    }

    ScriptManager& GlobalObjectManager::getScriptManager() const {
        return *scriptManager;
    }

    PhysicsSystem& GlobalObjectManager::getPhysicsSystem() const {
        return *physicsSystem;
    }

}



