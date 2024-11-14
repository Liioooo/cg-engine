#pragma once

#include "Entity.h"
#include "Asserts.h"

namespace CgEngine {

    class Scene;

    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        virtual void checkedDetachComponent(Entity entity, Scene& scene) = 0;
        virtual void detachAllComponents(Scene& scene) = 0;
        virtual void renderImGuiForEntity(Entity entity) = 0;
    };

    template<typename C>
    class ComponentArray : public IComponentArray {
    public:
        using ConstIterator = typename std::vector<C>::const_iterator;
        using Iterator = typename std::vector<C>::iterator;

    public:
        C& attachComponent(Entity entity) {
            CG_ASSERT(entityToComponentsIndex.find(entity) == entityToComponentsIndex.end(), "Component added to same entity more than once.")

            entityToComponentsIndex[entity] = components.size();
            componentsIndexToEntity[components.size()] = entity;
            return components.emplace_back(entity);
        }

        void detachComponent(Entity entity) {
            CG_ASSERT(entityToComponentsIndex.find(entity) != entityToComponentsIndex.end(), "Removing non-existent component.")

            size_t indexRemovedEntity = entityToComponentsIndex[entity];

            components[indexRemovedEntity] = std::move(components.back());
            components.pop_back();

            Entity entityLastElement = componentsIndexToEntity[components.size()];
            entityToComponentsIndex[entityLastElement] = indexRemovedEntity;
            componentsIndexToEntity[indexRemovedEntity] = entityLastElement;

            entityToComponentsIndex.erase(entity);
            componentsIndexToEntity.erase(components.size());
        }

        void checkedDetachComponent(Entity entity, Scene& scene) override {
            if (hasComponent(entity)) {
                components[entityToComponentsIndex[entity]].onDetach(scene);
                detachComponent(entity);
            }
        }

        void detachAllComponents(Scene& scene) override {
            entityToComponentsIndex.clear();
            componentsIndexToEntity.clear();
            for (auto& item: components) {
                item.onDetach(scene);
            }
        }

        C& getComponent(Entity entity) {
            CG_ASSERT(entityToComponentsIndex.find(entity) != entityToComponentsIndex.end(), "Getting non-existent component.")

            return components[entityToComponentsIndex[entity]];
        }

        bool hasComponent(Entity entity) const {
            return entityToComponentsIndex.find(entity) != entityToComponentsIndex.end();
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

    private:
        std::vector<C> components{};
        std::unordered_map<Entity, size_t> entityToComponentsIndex{};
        std::unordered_map<size_t , Entity> componentsIndexToEntity{};
    };

}
