#pragma once

#include "PxPhysicsAPI.h"

namespace CgEngine {

    class PhysicsMaterial {
    public:
        static PhysicsMaterial* createResource(const std::string& name);

        explicit PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);

        float getStaticFriction() const;
        float getDynamicFriction() const;
        float getRestitution() const;

        physx::PxMaterial* getPhysxMaterial();

    private:
        float staticFriction;
        float dynamicFriction;
        float restitution;

        physx::PxMaterial* physxMaterial;
    };

}
