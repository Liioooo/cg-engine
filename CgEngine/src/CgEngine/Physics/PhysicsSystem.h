#pragma once

#include "Scene/Entity.h"
#include "PxPhysicsAPI.h"
#include "PhysicsSimulationEventCallbacks.h"
#include "PhysicsControllerHitReportCallback.h"
#include "PhysicsCooking.h"

namespace CgEngine {

    namespace PhysXUtils {
        const physx::PxVec3& glmToPhysXVec(const glm::vec3& value);
        physx::PxExtendedVec3 glmToExtendedPhysXVec(const glm::vec3& value);
        const physx::PxVec4& glmToPhysXVec(const glm::vec4& value);
        physx::PxQuat glmToPhysXQuat(const glm::quat& value);

        glm::vec3 phsXToGlmVec(const physx::PxVec3& value);
        glm::vec3 phsXExtendedToGlmVec(const physx::PxExtendedVec3& value);
        glm::vec4 phsXToGlmVec(const physx::PxVec4& value);
        glm::quat phsXToGlmQuat(const physx::PxQuat& value);
    }

    class PhysXErrorCallback : public physx::PxErrorCallback {
    public:
        void reportError(physx::PxErrorCode::Enum code, const char *message, const char *file, int line) override;
    };

    struct PhysXSettings {
        const glm::vec3 gravity = {0.0f, -9.81f, 0.0f};
        const float simulateTimeStep = 1.0f / 60.0f; // seconds
        uint32_t solverIterations = 8;
        uint32_t solverVelocityIterations = 4;
    };

    enum class PhysicsCollisionDetection : uint32_t {
        Discrete, Continuous
    };

    enum class PhysicsForceMode {
        Force = physx::PxForceMode::eFORCE,
        Impulse = physx::PxForceMode::eIMPULSE,
        VelocityChange = physx::PxForceMode::eVELOCITY_CHANGE,
        Acceleration = physx::PxForceMode::eACCELERATION,
    };

    struct PhysicsRaycastHit {
        bool hitFound;
        Entity hitEntity;
        glm::vec3 hitPosition;
        float hitDistance;
    };

    class PhysicsSystem {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        physx::PxFoundation& getPhysxFoundation();
        physx::PxPhysics& getPhysxPhysics();
        physx::PxCpuDispatcher& getPhysxCpuDispatcher();
        physx::PxDefaultAllocator& getPhysxAllocator();
        PhysicsSimulationEventCallbacks& getSimulationEventCallbacks();
        PhysicsControllerHitReportCallback& getControllerHitReportCallback();
        PhysicsCooking& getPhysicsCooking();
        const PhysXSettings& getPhysxSettings() const;

        static physx::PxFilterFlags filterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock,physx::PxU32 constantBlockSize);

    private:
        physx::PxFoundation* physxFoundation;
        physx::PxPhysics* physxPhysics;
        physx::PxDefaultCpuDispatcher* physxCpuDispatcher;
        physx::PxDefaultAllocator physxAllocator;
        PhysXErrorCallback physxErrorCallback;
        PhysicsSimulationEventCallbacks simulationEventCallbacks;
        PhysicsControllerHitReportCallback controllerHitReportCallback;
        PhysicsCooking* physicsCooking;

        const PhysXSettings physxSettings;
    };

}
