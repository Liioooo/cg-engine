#pragma once

#include "PhysicsShape.h"

namespace CgEngine {

    class PhysicsActor;

    class PhysicsShapeSphere : public PhysicsShape {
    public:
        PhysicsShapeSphere(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, float radius, glm::vec3 offset, bool isTrigger);
    };

}
