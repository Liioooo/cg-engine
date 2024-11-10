#pragma once

#include "Events/Event.h"
#include "Entity.h"
#include "ComponentManager.h"
#include "TimeStep.h"
#include "Rendering/Texture.h"
#include "Physics/PhysicsScene.h"
#include "Rendering/CameraFrustum.h"

namespace CgEngine {

    class SceneRenderer;

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
        const TextureCube* irradianceMap;
        const TextureCube* prefilterMap;
        float environmentLod;
        float environmentIntensity;
    };

    class Scene {
    public:
        Scene(int viewportWidth, int viewportHeight);
        ~Scene();

        Entity createEntity(Entity parent);
        Entity createEntity(Entity parent, const std::string& id);
        void destroyEntity(Entity entity);
        Entity findEntityById(const std::string& id);
        const std::unordered_set<Entity>& getChildren(Entity entity);
        Entity getParent(Entity entity);
        bool hasParent(Entity entity) const;
        bool hasEntity(Entity entity) const;
        void setEntityTag(Entity entity, const std::string& tag);
        std::string getEntityTag(Entity entity);
        void updateTransforms();
        void onViewportResize(int width, int height);
        int getViewportWidth() const;
        int getViewportHeight() const;
        void submitPostUpdateFunction(std::function<void()>&& function);
        Uuid submitOnPreRenderFunction(const std::function<void(const CameraFrustum& camaraFrustum)>& function, bool once = false);
        void removeOnPreRenderFunction(Uuid uuid);
        Uuid submitOnRenderFunction(const std::function<void(SceneRenderer& renderer)>& function, bool once = false);
        void removeOnRenderFunction(Uuid uuid);
        void onUpdate(TimeStep ts);
        void onEvent(Event& event);
        void onRender(SceneRenderer& renderer);

        void executeFixedUpdate(TimeStep ts);

        template<typename C, typename P>
        C& attachComponent(Entity entity, P componentParams) {
            return componentManager->attachComponent<C>(entity, *this, componentParams);
        }

        template<typename C>
        void detachComponent(Entity entity) {
            componentManager->detachComponent<C>(entity, *this);
        }

        /**
         * References to Components must not be cached!!
         * Because if an other component of the same type is detached, the memory address of this component might change.
         * Which will lead to the reference pointing to a bad address!
         * @tparam C ComponentType
         * @param entity
         * @return reference to Component: auto& comp = s.getComponent<SomeComp>(entity);
         */
        template<typename C>
        C& getComponent(Entity entity) {
            return componentManager->getComponent<C>(entity);
        }

        template<typename C>
        bool hasComponent(Entity entity) const {
            return componentManager->hasComponent<C>(entity);
        }

        template<typename C>
        std::vector<Entity> getEntitiesWithComponent() {
            return componentManager->getEntitiesWithComponent<C>();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator begin() {
            return componentManager->begin<C>();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator end() {
            return componentManager->end<C>();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator cbegin() const {
            return componentManager->cbegin<C>();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator cend() const {
            return componentManager->cend<C>();
        }

        inline uint32_t getEntityCount() const {
            return entityCount;
        }

        PhysicsScene& getPhysicsScene() {
            return *physicsScene;
        }

    private:
        Entity nextEntityId = 1;
        uint32_t entityCount = 0;
        std::unordered_map<std::string, Entity> idToEntity{};
        std::unordered_map<Entity, std::string> entityTags{};
        std::unordered_map<Entity, std::unordered_set<Entity>> children{};
        std::unordered_map<Entity, Entity> parents{};
        std::vector<std::function<void()>> postUpdateFunctions{};

        template<typename T>
        struct FunctionCallback {
            Uuid uuid;
            bool once = false;
            T function;

            explicit FunctionCallback(const T& function, bool once = false) : function(function), once(once) {}
        };

        std::vector<FunctionCallback<std::function<void(const CameraFrustum& camaraFrustum)>>> onPreRenderFunctions{};
        std::vector<FunctionCallback<std::function<void(SceneRenderer& renderer)>>> onRenderFunctions{};

        ComponentManager* componentManager = new ComponentManager();
        int viewportWidth;
        int viewportHeight;
        PhysicsScene* physicsScene;

        CameraComponent& getPrimaryCamaraComponent();

        void recursiveDestroyEntity(Entity entity);
        void recursiveUpdateChildTransforms(Entity entity, const glm::mat4& parentModelMatrix, bool parentDirty);
        void executePostUpdateFunctions();
        void executeOnPreRenderFunctions(SceneRenderer& renderer);
        void executeOnRenderFunctions(SceneRenderer& renderer);
    };

}
