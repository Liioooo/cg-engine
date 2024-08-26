#pragma once

#include "Scene/Entity.h"

namespace CgEngine {

    class Scene;

    enum class PhysicsActorType {
        Controller, Actor
    };

    class AbstractPhysicsActor {
    public:
        AbstractPhysicsActor(Scene* scene, Entity entity);

        Scene& getScene() const;
        Entity getEntity() const;

        virtual PhysicsActorType getPhysicsActorType() const = 0;

    private:
        Scene* scene;
        Entity entity;

    };

}
