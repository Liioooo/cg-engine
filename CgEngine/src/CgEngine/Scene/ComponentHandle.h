#pragma once

#include "Entity.h"
#include "ComponentManager.h"

namespace CgEngine {

    template<typename C>
    class ComponentHandle {
    public:
        ComponentHandle() : componentManager(nullptr), owningEntity(NoEntity) {}
        ComponentHandle(ComponentManager* compManager, Entity entity) : componentManager(compManager), owningEntity(entity) {}

        bool isValid() const {
            return componentManager != nullptr && componentManager->hasComponent<C>(owningEntity);
        }

        C* operator->() {
            return &componentManager->getComponent<C>(owningEntity);
        }

        const C* operator->() const {
            return &componentManager->getComponent<C>(owningEntity);
        }

    private:
        ComponentManager* componentManager;
        Entity owningEntity;
    };

}
