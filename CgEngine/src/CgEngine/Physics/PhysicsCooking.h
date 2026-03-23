#pragma once

#include "PxPhysicsAPI.h"
#include "PhysicsTriangleMesh.h"
#include "PhysicsConvexMesh.h"

namespace CgEngine {

    class PhysicsCooking {
    public:
        PhysicsCooking() = default;
        explicit PhysicsCooking(physx::PxFoundation* pxFoundation, const physx::PxTolerancesScale& tolerancesScale);
        ~PhysicsCooking();

        PhysicsCooking(PhysicsCooking&& other) noexcept;
        PhysicsCooking& operator=(PhysicsCooking&& other) noexcept;

        PhysicsCooking(PhysicsCooking& other) = delete;
        PhysicsCooking& operator=(PhysicsCooking& other) = delete;

        PhysicsTriangleMesh* cookTriangleMesh(glm::vec3* vertices, uint32_t numVertices, uint32_t* indices, uint32_t indexCount);
        PhysicsConvexMesh* cookConvexMesh(glm::vec3* vertices, uint32_t numVertices);

    private:
        physx::PxCooking* physxCooking = nullptr;
    };

}
