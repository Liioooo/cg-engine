#include "NativeScript.h"
#include "Scene/Scene.h"
#include "Rendering/SceneRenderer.h"
#include "Scene/PrefabManager.h"
#include "Application.h"
#include "Components/ScriptComponent.h"

namespace CgEngine {
    EntityHandle NativeScript::getOwingEntity() {
        return {owningScene, owningEntity};
    }

    EntityHandle NativeScript::getParentEntity() {
        return getOwingEntity().getParent();
    }

    EntityHandle NativeScript::findEntityById(const std::string& id) {
        return owningScene->findEntityById(id);
    }

    EntityHandle NativeScript::createEntity() {
        return owningScene->createEntity(owningEntity);
    }

    EntityHandle NativeScript::createEntity(Entity parent) {
        return owningScene->createEntity(parent);
    }

    EntityHandle NativeScript::instantiatePrefab(const std::string& prefabName, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        return {owningScene,  PrefabManager::instantiatePrefab(owningScene, prefabName, owningEntity, position, rotation, scale, tag, id)};
    }

    EntityHandle NativeScript::instantiatePrefab(const std::string& prefabName, Entity parent, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const std::string& tag, const std::string& id) {
        return {owningScene, PrefabManager::instantiatePrefab(owningScene, prefabName, parent, position, rotation, scale, tag, id)};
    }

    EntityHandle NativeScript::getPrimaryCameraEntity() {
        return {owningScene, owningScene->getPrimaryCamaraComponent().getEntity()};
    }

    ComponentHandle<CameraComponent> NativeScript::getPrimaryCamaraComponent() {
        return owningScene->getPrimaryCameraComponentHandle();
    }

    void NativeScript::addUiElementClickListener(UiElement& uiElement, const std::function<void()>& cb) {
        uiElementCallbackConnections.emplace_back(uiElement.addClickListener(cb));
    }

    void NativeScript::setActiveScene(const std::string& name) {
        Application::get().getSceneManager().setActiveScene(name);
    }

    PhysicsRaycastHit NativeScript::physicsRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, const std::unordered_set<Entity>& excludeEntities) {
        return owningScene->getPhysicsScene().raycast(origin, direction, maxDistance, excludeEntities);
    }

    void NativeScript::drawDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
        owningScene->executeOnRender([from, to, color](SceneRenderer& renderer) {
            renderer.submitDebugLine(from, to, color);
        });
    }

    void NativeScript::executeOnRender(const std::function<void(SceneRenderer &)> &function) {
        owningScene->executeOnRender(function);
    }

    const ScriptParameterMap& NativeScript::getParameterMap() {
        return owningScene->getComponent<ScriptComponent>(owningEntity)->getParameterMap();
    }
}
