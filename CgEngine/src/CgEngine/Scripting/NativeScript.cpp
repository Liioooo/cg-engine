#include "NativeScript.h"
#include "Scene/Scene.h"
#include "Rendering/SceneRenderer.h"
#include "GlobalObjectManager.h"

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

    void NativeScript::setEntityTag(CgEngine::Entity entity, const std::string& tag) {
        owningScene->setEntityTag(entity, tag);
    }

    std::string NativeScript::getEntityTag(CgEngine::Entity entity) {
        return owningScene->getEntityTag(entity);
    }

    Entity NativeScript::createEntity(Entity parent) {
        return owningScene->createEntity(parent);
    }

    void NativeScript::setActiveScene(const std::string& name) {
        GlobalObjectManager::getInstance().getSceneManager().setActiveScene(name);
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
