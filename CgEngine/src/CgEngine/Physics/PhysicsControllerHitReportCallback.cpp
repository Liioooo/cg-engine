#include "Components/ScriptComponent.h"
#include "PhysicsControllerHitReportCallback.h"
#include "AbstractPhysicsActor.h"
#include "Scene/Scene.h"

namespace CgEngine {
    void PhysicsControllerHitReportCallback::onShapeHit(const physx::PxControllerShapeHit& hit) {
        auto* controllerActor = static_cast<AbstractPhysicsActor*>(hit.controller->getUserData());
        auto* otherActor = static_cast<AbstractPhysicsActor*>(hit.actor->userData);

        if (controllerActor->getScene().hasComponent<ScriptComponent>(controllerActor->getEntity())) {
            controllerActor->getScene().getComponent<ScriptComponent>(controllerActor->getEntity()).onCollisionEnter(otherActor->getEntity());
        }
        if (otherActor->getScene().hasComponent<ScriptComponent>(otherActor->getEntity())) {
            otherActor->getScene().getComponent<ScriptComponent>(otherActor->getEntity()).onCollisionEnter(controllerActor->getEntity());
        }
    }

    void PhysicsControllerHitReportCallback::onControllerHit(const physx::PxControllersHit& hit) {}

    void PhysicsControllerHitReportCallback::onObstacleHit(const physx::PxControllerObstacleHit& hit) {}
}