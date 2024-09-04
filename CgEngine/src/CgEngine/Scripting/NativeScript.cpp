#include <Application.h>
#include "NativeScript.h"
#include "Scene/Scene.h"
#include "Rendering/SceneRenderer.h"
#include "Scene/PrefabManager.h"

namespace CgEngine {
    void NativeScript::setScene(Scene* scene) {
        owningScene = scene;
    }

    void NativeScript::setEntity(Entity entity) {
        owningEntity = entity;
    }

    Entity NativeScript::getOwingEntity() {
        return owningEntity;
    }

    void NativeScript::destroyEntity(Entity entity) {
        auto& scene = owningScene;

        scene->submitPostUpdateFunction([entity, scene]() {
            scene->destroyEntity(entity);
        });
    }

    Entity NativeScript::findEntityById(const std::string& id) {
        return owningScene->findEntityById(id);
    }

    Entity NativeScript::getParentEntity() {
        return owningScene->getParent(owningEntity);
    }

    Entity NativeScript::getParentEntity(Entity entity) {
        return owningScene->getParent(entity);
    }

    const std::unordered_set<Entity>& NativeScript::getChildEntities() {
        return owningScene->getChildren(owningEntity);
    }

    const std::unordered_set<Entity>& NativeScript::getChildEntities(Entity entity) {
        return owningScene->getChildren(entity);
    }

    Entity NativeScript::createEntity() {
        return owningScene->createEntity(owningEntity);
    }

    Entity NativeScript::createEntity(Entity parent) {
        return owningScene->createEntity(parent);
    }

    Entity NativeScript::instantiatePrefab(const std::string& prefabName, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        return PrefabManager::instantiatePrefab(owningScene, prefabName, owningEntity, position, rotation, scale, tag, id);
    }

    Entity NativeScript::instantiatePrefab(const std::string& prefabName, Entity parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        return PrefabManager::instantiatePrefab(owningScene, prefabName, parent, position, rotation, scale, tag, id);
    }

    void NativeScript::setEntityTag(Entity entity, const std::string& tag) {
        owningScene->setEntityTag(entity, tag);
    }

    std::string NativeScript::getEntityTag(Entity entity) {
        return owningScene->getEntityTag(entity);
    }

    void NativeScript::setActiveScene(const std::string& name) {
        Application::get().getSceneManager().setActiveScene(name);
    }

    PhysicsRaycastHit NativeScript::physicsRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, const std::unordered_set<Entity>& excludeEntities) {
        return owningScene->getPhysicsScene().raycast(origin, direction, maxDistance, excludeEntities);
    }

    void NativeScript::drawDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
        owningScene->submitOnRenderFunction([from, to, color](SceneRenderer& renderer) {
            renderer.submitDebugLine(from, to, color);
        });
    }
}
