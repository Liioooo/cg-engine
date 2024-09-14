#pragma once

#include <XMLFile.h>
#include "PxPhysicsAPI.h"

namespace CgEngine {

    class PhysicsMaterial : public Resource {
    public:
        static PhysicsMaterial* createResource(const std::string& name);

        explicit PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);
        ~PhysicsMaterial() override;

        float getStaticFriction() const;
        float getDynamicFriction() const;
        float getRestitution() const;

        physx::PxMaterial* getPhysxMaterial();

    private:
        float staticFriction;
        float dynamicFriction;
        float restitution;

        physx::PxMaterial* physxMaterial;

        static inline XMLFile physicsMaterialsXML;
    };

}
