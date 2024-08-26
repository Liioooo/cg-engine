#pragma once

#include "PxPhysicsAPI.h"

namespace CgEngine {

    class PhysicsControllerHitReportCallback : public physx::PxUserControllerHitReport {
    public:
        void onShapeHit(const physx::PxControllerShapeHit& hit) override;

        void onControllerHit(const physx::PxControllersHit& hit) override;

        void onObstacleHit(const physx::PxControllerObstacleHit& hit) override;
    };

}
