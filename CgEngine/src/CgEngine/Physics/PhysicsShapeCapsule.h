#pragma once

#include "PhysicsShape.h"

namespace CgEngine {

    class PhysicsActor;

    class PhysicsShapeCapsule : public PhysicsShape {
    public:
        PhysicsShapeCapsule(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, float radius, float halfHeight, glm::vec3 offset, bool isTrigger);
    };

}
