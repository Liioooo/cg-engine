#include "PhysicsShape.h"

namespace CgEngine {
    PhysicsShape::PhysicsShape() {}

    void PhysicsShape::detachFromActor(physx::PxRigidActor* actor) {
        actor->detachShape(*physxShape);
    }
}
