#include "ScriptComponent.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void ScriptComponent::onAttach(Scene &scene, ScriptComponentParams &params) {
        script = GlobalObjectManager::getInstance().getScriptManager().getScriptInstance(params.scriptName);
        script->setEntity(entity);
        script->setScene(&scene);
        script->onAttach();
    }

    void ScriptComponent::onDetach(Scene& scene) {
        script->onDetach();
    }

    void ScriptComponent::fixedUpdate(TimeStep ts) {
        script->fixedUpdate(ts);
    }

    void ScriptComponent::update(TimeStep ts) {
        script->update(ts);
    }

    void ScriptComponent::lateUpdate(TimeStep ts) {
        script->lateUpdate(ts);
    }

    void ScriptComponent::onCollisionEnter(Entity other) {
        script->onCollisionEnter(other);
    }

    void ScriptComponent::onCollisionExit(Entity other) {
        script->onCollisionExit(other);
    }

    void ScriptComponent::onTriggerEnter(Entity other) {
        script->onTriggerEnter(other);
    }

    void ScriptComponent::onTriggerExit(Entity other) {
        script->onTriggerExit(other);
    }

    void ScriptComponent::onEvent(CgEngine::Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>(EVENT_BIND_FN(script->onMouseScrolled));
        dispatcher.dispatch<MouseButtonPressedEvent>(EVENT_BIND_FN(script->onMouseButtonPressed));
        dispatcher.dispatch<MouseMovedEvent>(EVENT_BIND_FN(script->onMouseMoved));
        dispatcher.dispatch<KeyPressedEvent>(EVENT_BIND_FN(script->onKeyPressed));
    }
}
