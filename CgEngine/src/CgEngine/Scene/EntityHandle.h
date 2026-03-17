#pragma once

#include "ComponentHandle.h"
#include "Entity.h"
#include "Scene.h"
#include "Asserts.h"

namespace CgEngine {

    class EntityHandle {
        friend struct std::hash<EntityHandle>;
    public:
        EntityHandle() : scene(nullptr), entity(NoEntity) {}
        EntityHandle(Scene* scene, Entity entity) : scene(scene), entity(entity) {}

        bool isValid() const;

        operator Entity() const {
            return entity;
        }

        bool operator==(const EntityHandle& other) const {
            return scene == other.scene && entity == other.entity;
        }

        void destroy();
        std::optional<std::string> getId() const;
        bool hasParent() const;
        EntityHandle getParent() const;
        std::unordered_set<EntityHandle> getChildren() const;
        EntityHandle getFirstChild() const;
        void setTag(const std::string& tag);
        std::string getTag() const;

        template<typename C>
        bool hasComponent() const {
            CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
            return scene->hasComponent<C>(entity);
        }

        template<typename C>
        ComponentHandle<C> getComponent() const {
            CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
            return scene->getComponent<C>(entity);
        }

        template<typename C>
        ComponentHandle<C> attachComponent(typename C::Params componentParams) {
            CG_ASSERT(isValid(), "Invalid EntityHandle. Entity: " + std::to_string(entity))
            return scene->attachComponent<C>(entity, componentParams);
        }

    private:
        Scene* scene;
        Entity entity;
    };

}

namespace std {
    template<>
    struct hash<CgEngine::EntityHandle> {
        size_t operator()(CgEngine::EntityHandle const& e) const noexcept {
            size_t h1 = std::hash<CgEngine::Scene*>{}(e.scene);
            size_t h2 = std::hash<CgEngine::Entity>{}(e.entity);
            return h1 ^ (h2 << 1);
        }
    };
}
