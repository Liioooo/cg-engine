#pragma once

#include "Scripting/ScriptManager.h"
#include "Scene/SceneManager.h"
#include "Resources/ResourceManager.h"
#include "Physics/PhysicsSystem.h"

namespace CgEngine {

    class SceneManager;

    class GlobalObjectManager {
    public:
        static GlobalObjectManager& getInstance();

        ~GlobalObjectManager();

        SceneManager& getSceneManager() const;
        ResourceManager& getResourceManager() const;
        ScriptManager& getScriptManager() const;
        PhysicsSystem& getPhysicsSystem() const;

    private:
        GlobalObjectManager();

        SceneManager* sceneManager;
        ResourceManager* resourceManager;
        ScriptManager* scriptManager;
        PhysicsSystem* physicsSystem;
    };
}



