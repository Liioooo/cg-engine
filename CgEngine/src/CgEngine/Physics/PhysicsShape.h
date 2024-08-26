#pragma once

#include "PxPhysicsAPI.h"
#include "PhysicsMaterial.h"

namespace CgEngine {

    class PhysicsShape {
    public:
        virtual ~PhysicsShape() = default;

        void detachFromActor(physx::PxRigidActor* actor);

    protected:
        explicit PhysicsShape();

        physx::PxShape* physxShape;
    };

}
