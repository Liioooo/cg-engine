#pragma once

#include "PhysicsShape.h"

namespace CgEngine {

    class PhysicsActor;

    class PhysicsShapeBox : public PhysicsShape {
    public:
        PhysicsShapeBox(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger);
    };

}
