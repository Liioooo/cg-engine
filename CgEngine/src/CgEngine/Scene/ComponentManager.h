#pragma once

#include "Entity.h"
#include "Components/TransformComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/MeshRendererComponent.h"
#include "Components/AnimatedMeshRendererComponent.h"
#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyboxComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CapsuleColliderComponent.h"
#include "Components/TriangleColliderComponent.h"
#include "Components/ConvexColliderComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/CharacterControllerComponent.h"
#include "Components/UiCanvasComponent.h"
#include "ComponentArray.h"

namespace CgEngine {

    class ComponentManager {
    public:
        ComponentManager() {
            registerComponentType<TransformComponent>();
            registerComponentType<ScriptComponent>();
            registerComponentType<MeshRendererComponent>();
            registerComponentType<AnimatedMeshRendererComponent>();
            registerComponentType<CameraComponent>();
            registerComponentType<DirectionalLightComponent>();
            registerComponentType<PointLightComponent>();
            registerComponentType<SpotLightComponent>();
            registerComponentType<SkyboxComponent>();
            registerComponentType<BoxColliderComponent>();
            registerComponentType<SphereColliderComponent>();
            registerComponentType<CapsuleColliderComponent>();
            registerComponentType<TriangleColliderComponent>();
            registerComponentType<ConvexColliderComponent>();
            registerComponentType<RigidBodyComponent>();
            registerComponentType<CharacterControllerComponent>();
            registerComponentType<CharacterControllerComponent>();
            registerComponentType<UiCanvasComponent>();
        }

        template<typename C>
        void attachComponent(C& component) {
            getComponentArray<C>()->attachComponent(component);
        }

        template<typename C>
        void detachComponent(Entity entity) {
            getComponentArray<C>()->detachComponent(entity);
        }

        template<typename C>
        C& getComponent(Entity entity) {
            return getComponentArray<C>()->getComponent(entity);
        }

        template<typename C>
        bool hasComponent(Entity entity) const {
            return getComponentArray<C>()->hasComponent(entity);
        }

        template<typename C>
        std::vector<Entity> getEntitiesWithComponent() {
            return getComponentArray<C>()->getAllEntities();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator begin() {
            return getComponentArray<C>()->begin();
        }

        template<typename C>
        typename ComponentArray<C>::Iterator end() {
            return getComponentArray<C>()->end();
        }

        template<typename C>
        typename ComponentArray<C>::ConstIterator cbegin() const {
            return getComponentArray<C>()->cbegin();
        }

        template<typename C>
        typename ComponentArray<C>::ConstIterator cend() const {
            return getComponentArray<C>()->cend();
        }

        void destroyEntity(Entity entity, Scene& scene) {
            for (const auto &pair: componentArrays) {
                auto const& compArray = pair.second;
                compArray->checkedDetachComponent(entity, scene);
            }
        }

    private:
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};

        template<typename C>
        void registerComponentType() {
            const char* typeName = typeid(C).name();
            componentArrays.insert({typeName, std::make_shared<ComponentArray<C>>()});
        }

        template<typename C>
        std::shared_ptr<ComponentArray<C>> getComponentArray() const {
            const char* typeName = typeid(C).name();
            return std::static_pointer_cast<ComponentArray<C>>(componentArrays.at(typeName));
        }
    };

}


