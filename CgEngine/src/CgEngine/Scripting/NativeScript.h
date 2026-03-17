#pragma once

#include "Scene/Scene.h"
#include "Events/MouseScrolledEvent.h"
#include "Events/MouseButtonPressedEvent.h"
#include "Events/MouseMovedEvent.h"
#include "Events/KeyPressedEvent.h"
#include "Physics/PhysicsSystem.h"
#include "Application.h"
#include "Scene/ComponentHandle.h"
#include "Scene/EntityHandle.h"
#include "ScriptParameterMap.h"

namespace CgEngine {

    class NativeScript {
    public:
        friend class ScriptComponent;

    protected:
        NativeScript() = default;
        ~NativeScript() = default;

        virtual void onAttach() {};
        virtual void onEnable() {};
        virtual void onDetach() {};

        virtual void fixedUpdate(TimeStep ts) {};
        virtual void update(TimeStep ts) {};
        virtual void lateUpdate(TimeStep ts) {};

        virtual void onCollisionEnter(EntityHandle other) {}
        virtual void onCollisionExit(EntityHandle other) {}

        virtual void onTriggerEnter(EntityHandle other) {};
        virtual void onTriggerExit(EntityHandle other) {};

        virtual void onMouseScrolled(MouseScrolledEvent& event) {}
        virtual void onMouseButtonPressed(MouseButtonPressedEvent& event) {}
        virtual void onMouseMoved(MouseMovedEvent& event) {}
        virtual void onKeyPressed(KeyPressedEvent& event) {}

        virtual void onRenderImGui() {};

        EntityHandle getOwingEntity();
        EntityHandle createEntity();
        EntityHandle findEntityById(const std::string& id);
        EntityHandle createEntity(Entity parent);
        EntityHandle instantiatePrefab(const std::string& prefabName, glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 rotation = {0.0f, 0.0f, 0.0f}, glm::vec3 scale = {1.0f, 1.0f, 1.0f}, const std::string& tag = "", const std::string& id = "");
        EntityHandle instantiatePrefab(const std::string& prefabName, Entity parent, glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 rotation = {0.0f, 0.0f, 0.0f}, glm::vec3 scale = {1.0f, 1.0f, 1.0f}, const std::string& tag = "", const std::string& id = "");
        CameraComponent& getPrimaryCamaraComponent();

        void addUiElementClickListener(UiElement& uiElement, const std::function<void()>& cb);

        void setActiveScene(const std::string& name);

        PhysicsRaycastHit physicsRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, const std::unordered_set<Entity>& excludeEntities);

        void drawDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);

        const ScriptParameterMap& getParameterMap();

        template<typename R>
        ResRef<R> getResource(const std::string& name) {
            return Application::get().getResourceManager().getResource<R>(name);
        }

        template<typename R, typename S>
        ResRef<R> getResource(const std::string& name, const S& spec) {
            return Application::get().getResourceManager().getResource<R>(name, spec);
        }

    private:
        Scene* owningScene;
        Entity owningEntity;

        std::vector<CallbackConnection<std::function<void()>>> uiElementCallbackConnections;
    };

}
