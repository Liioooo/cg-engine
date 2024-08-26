#include "PhysicsSystem.h"
#include "Asserts.h"

namespace CgEngine {
    namespace PhysXUtils {
        const physx::PxVec3& glmToPhysXVec(const glm::vec3& value) {
            return *(physx::PxVec3*)(&value);
        }

        const physx::PxVec4& glmToPhysXVec(const glm::vec4& value) {
            return *(physx::PxVec4*)(&value);
        }

        physx::PxExtendedVec3 glmToExtendedPhysXVec(const glm::vec3& value) {
            return {value.x, value.y, value.z};
        }

        physx::PxQuat glmToPhysXQuat(const glm::quat& value) {
            return {value.x, value.y, value.z, value.w};
        }

        glm::vec3 phsXToGlmVec(const physx::PxVec3& value) {
            return *(glm::vec3*)&value;
        }

        glm::vec3 phsXExtendedToGlmVec(const physx::PxExtendedVec3& value) {
            return {value.x, value.y, value.z};
        }

        glm::vec4 phsXToGlmVec(const physx::PxVec4& value) {
            return *(glm::vec4 *)&value;
        }

        glm::quat phsXToGlmQuat(const physx::PxQuat& value) {
            return {value.w, value.x, value.y, value.z};
        }
    }

    void PhysXErrorCallback::reportError(physx::PxErrorCode::Enum code, const char *message, const char *file, int line) {
        const char* error;

        switch (code) {
            case physx::PxErrorCode::eNO_ERROR:
                error = "";
                break;
            case physx::PxErrorCode::eDEBUG_INFO:
                error = "Info";
                break;
            case physx::PxErrorCode::eDEBUG_WARNING:
                error = "Warning";
                break;
            case physx::PxErrorCode::eINVALID_PARAMETER:
                error = "Invalid Parameter";
                break;
            case physx::PxErrorCode::eINVALID_OPERATION:
                error = "Invalid Operation";
                break;
            case physx::PxErrorCode::eOUT_OF_MEMORY:
                error = "Out Of Memory";
                break;
            case physx::PxErrorCode::eINTERNAL_ERROR:
                error = "Internal Error";
                break;
            case physx::PxErrorCode::eABORT:
                error = "Abort";
                break;
            case physx::PxErrorCode::ePERF_WARNING:
                error = "Performance Warning";
                break;
            case physx::PxErrorCode::eMASK_ALL:
                error = "Unknown Error";
                break;
        }

        CG_LOGGING_ERROR("PhysX: [{0}]: {1} at: {2}, {3}", error, message, file, line)
    }

    PhysicsSystem::PhysicsSystem() {
        physxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, physxAllocator, physxErrorCallback);
        CG_ASSERT(physxFoundation, "Error while creating PhysXFoundation")

        physx::PxTolerancesScale tolerancesScale = physx::PxTolerancesScale();
        tolerancesScale.length = 1.0f;
        tolerancesScale.speed = 100.0f;

        physx::PxPvd* pvd = nullptr;

#ifdef _DEBUG
        pvd = physx::PxCreatePvd(*physxFoundation);
        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 1000);
        pvd->connect(*transport,physx::PxPvdInstrumentationFlag::eALL);
#endif

        physxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *physxFoundation, tolerancesScale, false, pvd);
        CG_ASSERT(physxPhysics, "Error while creating PhysXPhysics")

        physicsCooking = new PhysicsCooking(physxFoundation, tolerancesScale);

        physxCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
    }

    PhysicsSystem::~PhysicsSystem() {
        physxCpuDispatcher->release();
        physxCpuDispatcher = nullptr;

        physxPhysics->release();
        physxPhysics = nullptr;

        delete physicsCooking;

        physxFoundation->release();
        physxFoundation = nullptr;
    }

    physx::PxFoundation& PhysicsSystem::getPhysxFoundation() {
        return *physxFoundation;
    }

    physx::PxPhysics& PhysicsSystem::getPhysxPhysics() {
        return *physxPhysics;
    }

    physx::PxCpuDispatcher& PhysicsSystem::getPhysxCpuDispatcher() {
        return *physxCpuDispatcher;
    }

    physx::PxDefaultAllocator& PhysicsSystem::getPhysxAllocator() {
        return physxAllocator;
    }

    PhysicsSimulationEventCallbacks& PhysicsSystem::getSimulationEventCallbacks() {
        return simulationEventCallbacks;
    }

    PhysicsControllerHitReportCallback& PhysicsSystem::getControllerHitReportCallback() {
        return controllerHitReportCallback;
    }

    PhysicsCooking& PhysicsSystem::getPhysicsCooking() {
        return *physicsCooking;
    }

    const PhysXSettings &PhysicsSystem::getPhysxSettings() const {
        return physxSettings;
    }

    physx::PxFilterFlags PhysicsSystem::filterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize) {
        if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1)) {
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
            return physx::PxFilterFlag::eDEFAULT;
        }

        pairFlags = physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;

        if (!physx::PxFilterObjectIsKinematic(attributes0) && !physx::PxFilterObjectIsKinematic(attributes1)) {
            pairFlags |= physx::PxPairFlag::eSOLVE_CONTACT;
        }

        if (filterData0.word0 == static_cast<uint32_t>(PhysicsCollisionDetection::Continuous) || filterData1.word0 == static_cast<uint32_t>(PhysicsCollisionDetection::Continuous)) {
            pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
        }

        pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
        pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;

        return physx::PxFilterFlag::eDEFAULT;
    }
}
