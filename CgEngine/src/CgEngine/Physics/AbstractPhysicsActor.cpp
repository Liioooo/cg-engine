#include "AbstractPhysicsActor.h"

namespace CgEngine {
    AbstractPhysicsActor::AbstractPhysicsActor(Scene* scene, Entity entity) : scene(scene), entity(entity) {}

    Scene& AbstractPhysicsActor::getScene() const {
        return *scene;
    }

    Entity AbstractPhysicsActor::getEntity() const {
        return entity;
    }
}
