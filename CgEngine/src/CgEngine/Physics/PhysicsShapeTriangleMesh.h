#pragma once

#include "PhysicsShape.h"
#include "PhysicsTriangleMesh.h"

namespace CgEngine {

    class PhysicsActor;

    class PhysicsShapeTriangleMesh : public PhysicsShape {
    public:
        PhysicsShapeTriangleMesh(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, PhysicsTriangleMesh& physicsMesh, bool isTrigger);
    };

}
