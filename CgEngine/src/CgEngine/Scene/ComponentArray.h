#pragma once

#include "Entity.h"
#include "Asserts.h"

namespace CgEngine {

    class Scene;

    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        virtual void checkedDetachComponent(Entity entity, Scene& scene) = 0;
        virtual void detachAllComponentsInstantly(Scene& scene) = 0;
        virtual void renderImGuiForEntity(Entity entity) = 0;
        virtual bool executePendingOperations(Scene& scene) = 0;
        virtual void callOnEnableForAddedComponents(Scene& scene) = 0;
    };

    template<typename C>
    class ComponentArray : public IComponentArray {
    public:
        using ConstIterator = typename std::vector<C>::const_iterator;
        using Iterator = typename std::vector<C>::iterator;

        void attachComponent(Entity entity, Scene& scene, typename C::Params& componentParams) {
            CG_ASSERT(entityToComponentsIndex.find(entity) == entityToComponentsIndex.end(), "Component added to same entity more than once.")
            CG_ASSERT(componentsPendingAdd.find(entity) == componentsPendingAdd.end(), "Component added to same entity more than once.")

            componentsPendingAdd.emplace(entity, C(entity));
            componentsPendingAdd.at(entity).onAttach(scene, componentParams);
        }

        void detachComponent(Entity entity) {
            CG_ASSERT(entityToComponentsIndex.find(entity) != entityToComponentsIndex.end(), "Removing non-existent component.")
            componentsPendingRemove.push_back(entity);
        }

        void checkedDetachComponent(Entity entity, Scene& scene) override {
            if (hasComponent(entity)) {
                detachComponent(entity);
            }
        }

        void detachAllComponentsInstantly(Scene& scene) override {
            entityToComponentsIndex.clear();
            componentsIndexToEntity.clear();
            for (auto& item: components) {
                item.onDetach(scene);
            }
            for (auto& [entity, component] : componentsPendingAdd) {
                component.onDetach(scene);
            }
        }

        C& getComponent(Entity entity) {
            CG_ASSERT(entityToComponentsIndex.find(entity) != entityToComponentsIndex.end() || componentsPendingAdd.find(entity) != componentsPendingAdd.end(), "Getting non-existent component. Entity: " + std::to_string(entity) + ", Component: " + typeid(C).name())

            if (entityToComponentsIndex.find(entity) != entityToComponentsIndex.end()) {
                return components[entityToComponentsIndex[entity]];
            }
            return componentsPendingAdd.at(entity);

        }

        bool hasComponent(Entity entity) const {
            return entityToComponentsIndex.find(entity) != entityToComponentsIndex.end() || componentsPendingAdd.find(entity) != componentsPendingAdd.end();
        }

        std::vector<Entity> getAllEntities() {
            std::vector<Entity> entities;
            entities.reserve(entityToComponentsIndex.size());
            for (const auto &item : entityToComponentsIndex) {
                entities.push_back(item.first);
            }
            return entities;
        }

        ConstIterator cbegin() const {
            return components.cbegin();
        }

        ConstIterator cend() const {
            return components.cend();
        }

        Iterator begin() {
            return components.begin();
        }

        Iterator end() {
            return components.end();
        }

        void renderImGuiForEntity(Entity entity) override {
            if (!hasComponent(entity)) {
                return;
            }
            getComponent(entity).onRenderImGui();
        }

        bool executePendingOperations(Scene& scene) override {
            for (auto& [entity, component] : componentsPendingAdd) {
                entityToComponentsIndex[entity] = components.size();
                componentsIndexToEntity[components.size()] = entity;
                components.push_back(std::move(component));
            }

            for (auto& entity : componentsPendingRemove) {
                size_t indexRemovedEntity = entityToComponentsIndex[entity];
                components[indexRemovedEntity].onDetach(scene);

                components[indexRemovedEntity] = std::move(components.back());
                components.pop_back();

                Entity entityLastElement = componentsIndexToEntity[components.size()];
                entityToComponentsIndex[entityLastElement] = indexRemovedEntity;
                componentsIndexToEntity[indexRemovedEntity] = entityLastElement;

                entityToComponentsIndex.erase(entity);
                componentsIndexToEntity.erase(components.size());
            }
            componentsPendingRemove.clear();

            return !componentsPendingAdd.empty();
        }

        void callOnEnableForAddedComponents(Scene& scene) override {
            for (auto it = componentsPendingAdd.begin(); it != componentsPendingAdd.end(); ) {
                Entity entity = it->first;

                auto found = entityToComponentsIndex.find(entity);
                if (found != entityToComponentsIndex.end()) {
                    components[found->second].onEnable(scene);
                    it = componentsPendingAdd.erase(it);
                } else {
                    ++it;
                }
            }
        }

    private:
        std::vector<C> components{};
        std::unordered_map<Entity, C> componentsPendingAdd{};
        std::vector<Entity> componentsPendingRemove{};
        std::unordered_map<Entity, size_t> entityToComponentsIndex{};
        std::unordered_map<size_t, Entity> componentsIndexToEntity{};
    };

}
