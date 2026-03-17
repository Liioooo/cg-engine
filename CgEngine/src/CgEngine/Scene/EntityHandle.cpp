#include "EntityHandle.h"

namespace CgEngine {

    bool EntityHandle::isValid() const {
        return scene != nullptr && scene->hasEntity(entity);
    }

    void EntityHandle::destroy() {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        scene->destroyEntity(entity);
    }

    std::optional<std::string> EntityHandle::getId() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        return scene->getIdForEntity(entity);
    }

    bool EntityHandle::hasParent() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        return scene->hasParent(entity);
    }

    EntityHandle EntityHandle::getParent() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        return {scene, scene->getParent(entity)};
    }

    std::unordered_set<EntityHandle> EntityHandle::getChildren() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))

        const auto& children = scene->getChildren(entity);
        std::unordered_set<EntityHandle> childHandles;
        for (const auto& child : children) {
            childHandles.emplace(scene, child);
        }
        return childHandles;
    }

    EntityHandle EntityHandle::getFirstChild() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))

        const auto& children = scene->getChildren(entity);
        CG_ASSERT(!children.empty(), "Entity has no children. Entity: " + std::to_string(entity))

        return {scene, *children.begin()};
    }

    void EntityHandle::setTag(const std::string& tag) {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        scene->setEntityTag(entity, tag);
    }

    std::string EntityHandle::getTag() const {
        CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
        return scene->getEntityTag(entity);
    }


}
