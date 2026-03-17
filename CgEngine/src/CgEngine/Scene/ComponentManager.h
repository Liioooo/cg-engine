#pragma once

#include "Entity.h"
#include "ComponentArray.h"

namespace CgEngine {

    class ComponentManager {
    public:
        ComponentManager();

        template<typename C>
        void attachComponent(Entity entity, Scene& scene, typename C::Params& componentParams) {
            componentParams.verifyParams();
            getComponentArray<C>()->attachComponent(entity, scene, componentParams);
        }

        template<typename C>
        void detachComponent(Entity entity, Scene& scene) {
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

        void destroyAllComponents(Scene& scene) {
            for (const auto &pair: componentArrays) {
                pair.second->detachAllComponentsInstantly(scene);
            }
        }

        void renderImGuiForEntity(Entity entity) {
            for (const auto &pair: componentArrays) {
                pair.second->renderImGuiForEntity(entity);
            }
        }

        bool executePendingOperations(Scene& scene) {
            bool wereComponentsAdded = false;

            for (const auto &pair: componentArrays) {
                wereComponentsAdded |= pair.second->executePendingOperations(scene);
            }

            return wereComponentsAdded;
        }

        void callOnEnableForAddedComponents(Scene& scene) {
            for (const auto &pair: componentArrays) {
                pair.second->callOnEnableForAddedComponents(scene);
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


