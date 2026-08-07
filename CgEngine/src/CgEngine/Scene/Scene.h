#pragma once

#include "Events/Event.h"
#include "Entity.h"
#include "ComponentManager.h"
#include "TimeStep.h"
#include "Physics/PhysicsScene.h"
#include "Rendering/CameraFrustum.h"
#include "ComponentHandle.h"
#include "Rendering/DescriptorSet.h"
#include "Components/CameraComponent.h"

namespace CgEngine {

    class SceneRenderer;
    class EntityHandle;

    struct SceneSpotLight {
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
        float innerAngle;
        float outerAngle;
    };

    struct ScenePointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
    };

    struct SceneLightEnvironment {
        glm::vec3 dirLightDirection;
        float dirLightIntensity = 0.0f;
        glm::vec3 dirLightColor;
        bool dirLightCastShadows = false;
        std::vector<ScenePointLight> pointLights;
        std::vector<SceneSpotLight> spotLights;
    };

    struct SceneEnvironment {
        const DescriptorSet* environmentMapDescriptorSet = nullptr;
        float environmentLod;
        float environmentIntensity;
    };

    class Scene {
        friend class ImGuiSceneView;
        friend class EntityHandle;
    public:
        Scene(int viewportWidth, int viewportHeight);
        ~Scene();

        EntityHandle createEntity(Entity parent);
        EntityHandle createEntity(Entity parent, const std::string& id);
        EntityHandle findEntityById(const std::string& id);
        bool hasEntity(Entity entity) const;
        void updateTransforms();
        void onViewportResize(int width, int height);
        int getViewportWidth() const;
        int getViewportHeight() const;
        void executeOnRender(const std::function<void(SceneRenderer& renderer)>& function);
        void onUpdate(TimeStep ts);
        void onEvent(Event& event);
        void onRender(SceneRenderer& renderer);

        void executeFixedUpdate(TimeStep ts);
        void executeAllPendingOperations(bool forceUpdateTransforms);

        template<typename C>
        ComponentHandle<C> attachComponent(Entity entity, typename C::Params componentParams) {
            CG_ASSERT(hasEntity(entity), "Scene::attachComponent: Entity does not exist in the scene.")
            componentManager.attachComponent<C>(entity, *this, componentParams);
            return ComponentHandle<C>(&componentManager, entity);
        }

        template<typename C>
        void detachComponent(Entity entity) {
            componentManager.detachComponent<C>(entity, *this);
        }

        template<typename C>
        ComponentHandle<C> getComponent(Entity entity) {
            CG_ASSERT(hasEntity(entity), "Scene::getComponent: Entity does not exist in the scene.")
            CG_ASSERT(componentManager.hasComponent<C>(entity), "Scene::getComponent: Entity does not have the requested component.")
            return ComponentHandle<C>(&componentManager, entity);
        }

        template<typename C>
        bool hasComponent(Entity entity) const {
            return componentManager.hasComponent<C>(entity);
        }

        template<typename C>
        std::vector<Entity> getEntitiesWithComponent() {
            return componentManager.getEntitiesWithComponent<C>();
        }

        uint32_t getEntityCount() const {
            return children.size();
        }

        PhysicsScene& getPhysicsScene() {
            return physicsScene;
        }

        CameraComponent& getPrimaryCamaraComponent();
        ComponentHandle<CameraComponent> getPrimaryCameraComponentHandle();

    private:
        Entity nextEntityId = 1;
        std::unordered_map<std::string, Entity> idToEntity{};
        std::unordered_map<Entity, std::string> entityTags{};
        std::unordered_map<Entity, std::unordered_set<Entity>> children{};
        std::unordered_map<Entity, Entity> parents{};
        std::unordered_set<Entity> entitiesToBeDestroyed{};

        std::vector<std::function<void(SceneRenderer& renderer)>> onRenderFunctions{};

        ComponentManager componentManager;
        int viewportWidth;
        int viewportHeight;
        PhysicsScene physicsScene;

        void destroyEntity(Entity entity);
        std::optional<std::string> getIdForEntity(Entity entity) const;
        const std::unordered_set<Entity>& getChildren(Entity entity) const;
        Entity getParent(Entity entity) const;
        bool hasParent(Entity entity) const;
        void setEntityTag(Entity entity, const std::string& tag);
        std::string getEntityTag(Entity entity) const;

        void findRecursiveEntitiesToDestroy(Entity entity, std::unordered_set<Entity>& recursivelyDestroyedEntities);
        void recursiveUpdateChildTransforms(Entity entity, const glm::mat4& parentModelMatrix, bool parentDirty);
        void executeOnRenderFunctions(SceneRenderer& renderer);
    };

}
