#pragma once

#include "PhysicsShape.h"
#include "PhysicsConvexMesh.h"

namespace CgEngine {

    class PhysicsActor;

    class PhysicsShapeConvexMesh : public PhysicsShape {
    public:
        PhysicsShapeConvexMesh(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, PhysicsConvexMesh& physicsMesh, bool isTrigger);
    };

}
