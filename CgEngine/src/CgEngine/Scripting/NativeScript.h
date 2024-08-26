#pragma once

#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Events/MouseScrolledEvent.h"
#include "Events/MouseButtonPressedEvent.h"
#include "Events/MouseMovedEvent.h"
#include "Events/KeyPressedEvent.h"
#include "Physics/PhysicsSystem.h"

namespace CgEngine {

    class NativeScript {
    public:
        friend class ScriptComponent;

    protected:
        NativeScript() = default;
        ~NativeScript() = default;

        void setScene(Scene* scene);
        void setEntity(Entity entity);

        virtual void onAttach() {};
        virtual void onDetach() {};

        virtual void fixedUpdate(TimeStep ts) {};
        virtual void update(TimeStep ts) {};
        virtual void lateUpdate(TimeStep ts) {};

        virtual void onCollisionEnter(Entity other) {}
        virtual void onCollisionExit(Entity other) {}

        virtual void onTriggerEnter(Entity other) {};
        virtual void onTriggerExit(Entity other) {};

        virtual void onMouseScrolled(MouseScrolledEvent& event) {}
        virtual void onMouseButtonPressed(MouseButtonPressedEvent& event) {}
        virtual void onMouseMoved(MouseMovedEvent& event) {}
        virtual void onKeyPressed(KeyPressedEvent& event) {}

        Entity getOwingEntity();
        void destroyEntity(Entity entity);
        Entity findEntityById(const std::string& id);
        Entity getParentEntity();
        Entity getParentEntity(Entity entity);
        const std::unordered_set<Entity>& getChildEntities();
        const std::unordered_set<Entity>& getChildEntities(Entity entity);
        Entity createEntity();
        Entity createEntity(Entity parent);
        void setEntityTag(Entity entity, const std::string& tag);
        std::string getEntityTag(Entity entity);

        void setActiveScene(const std::string& name);

        PhysicsRaycastHit physicsRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, const std::unordered_set<Entity>& excludeEntities);

        void drawDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);

        template<typename C>
        C& getComponent() {
            return owningScene->getComponent<C>(owningEntity);
        }

        template<typename C>
        C& getComponent(Entity entity) {
            return owningScene->getComponent<C>(entity);
        }

        template<typename C>
        bool hasEntityComponent() {
            return owningScene->hasComponent<C>(owningEntity);
        }

        template<typename C>
        bool hasEntityComponent(Entity entity) {
            return owningScene->hasComponent<C>(entity);
        }

        template<typename C>
        C& attachComponent() {
            return owningScene->attachComponent<C>(owningEntity);
        }

        template<typename C, typename P>
        C& attachComponent(P componentPrams) {
            return owningScene->attachComponent<C>(owningEntity, componentPrams);
        }

        template<typename C, typename P>
        C& attachComponent(Entity entity, P componentPrams) {
            return owningScene->attachComponent<C>(entity, componentPrams);
        }

        template<typename C>
        C& attachComponent(Entity entity) {
            return owningScene->attachComponent<C>(entity);
        }

        template<typename C>
        void detachComponent() {
            owningScene->detachComponent<C>(owningEntity);
        }

        template<typename C>
        void detachComponent(Entity entity) {
            owningScene->detachComponent<C>(entity);
        }

    private:
        Scene* owningScene;
        Entity owningEntity;
    };

}
