#include "PhysicsCooking.h"
#include "GlobalObjectManager.h"
#include "Asserts.h"

namespace CgEngine {
    PhysicsCooking::PhysicsCooking(physx::PxFoundation* pxFoundation, const physx::PxTolerancesScale& tolerancesScale) {
        auto params = physx::PxCookingParams(tolerancesScale);
        params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
        params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
        params.meshWeldTolerance = 0.05f;
        physxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pxFoundation, params);
        CG_ASSERT(physxCooking, "Error while creating PhysXCooking")
    }

    PhysicsCooking::~PhysicsCooking() {
        physxCooking->release();
        physxCooking = nullptr;
    }

    PhysicsTriangleMesh* PhysicsCooking::cookTriangleMesh(glm::vec3* vertices, uint32_t numVertices, uint32_t* indices, uint32_t indexCount) {
        physx::PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = numVertices;
        meshDesc.points.stride = sizeof(glm::vec3);
        meshDesc.points.data = vertices;

        meshDesc.triangles.count = indexCount / 3;
        meshDesc.triangles.stride = 3 * sizeof(uint32_t);
        meshDesc.triangles.data = indices;

        physx::PxDefaultMemoryOutputStream writeBuffer;
        physx::PxTriangleMeshCookingResult::Enum result;
        bool status = physxCooking->cookTriangleMesh(meshDesc, writeBuffer, &result);

        CG_ASSERT(status, "Failed to cook TriangleMesh")

        auto& physxPhysics = GlobalObjectManager::getInstance().getPhysicsSystem().getPhysxPhysics();

        physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

        return new PhysicsTriangleMesh(physxPhysics.createTriangleMesh(readBuffer));
    }

    PhysicsConvexMesh* PhysicsCooking::cookConvexMesh(glm::vec3* vertices, uint32_t numVertices) {
        physx::PxConvexMeshDesc meshDesc;
        meshDesc.points.count = numVertices;
        meshDesc.points.stride = sizeof(glm::vec3);
        meshDesc.points.data = vertices;
        meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

        physx::PxDefaultMemoryOutputStream writeBuffer;
        physx::PxConvexMeshCookingResult::Enum result;
        bool status = physxCooking->cookConvexMesh(meshDesc, writeBuffer, &result);

        CG_ASSERT(status, "Failed to cook ConvexMesh")

        auto& physxPhysics = GlobalObjectManager::getInstance().getPhysicsSystem().getPhysxPhysics();

        physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

        return new PhysicsConvexMesh(physxPhysics.createConvexMesh(readBuffer));
    }
}