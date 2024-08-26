#include "AbstractPhysicsActor.h"

namespace CgEngine {
    AbstractPhysicsActor::AbstractPhysicsActor(CgEngine::Scene* scene, CgEngine::Entity entity) : scene(scene), entity(entity) {}

    Scene& AbstractPhysicsActor::getScene() const {
        return *scene;
    }

    Entity AbstractPhysicsActor::getEntity() const {
        return entity;
    }
}