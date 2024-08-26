#include "PhysicsSimulationEventCallbacks.h"
#include "AbstractPhysicsActor.h"
#include "Scene/Scene.h"

namespace CgEngine {
    void PhysicsSimulationEventCallbacks::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}

    void PhysicsSimulationEventCallbacks::onWake(physx::PxActor** actors, physx::PxU32 count) {}

    void PhysicsSimulationEventCallbacks::onSleep(physx::PxActor** actors, physx::PxU32 count) {}

    void PhysicsSimulationEventCallbacks::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
        auto removed0 = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
        auto removed1 = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

        if (removed0 || removed1) {
            return;
        }

        auto* actor0 = static_cast<AbstractPhysicsActor*>(pairHeader.actors[0]->userData);
        auto* actor1 = static_cast<AbstractPhysicsActor*>(pairHeader.actors[1]->userData);

        if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH) {
            if (actor0->getScene().hasComponent<ScriptComponent>(actor0->getEntity())) {
                actor0->getScene().getComponent<ScriptComponent>(actor0->getEntity()).onCollisionEnter(actor1->getEntity());
            }
            if (actor1->getScene().hasComponent<ScriptComponent>(actor1->getEntity())) {
                actor1->getScene().getComponent<ScriptComponent>(actor1->getEntity()).onCollisionEnter(actor0->getEntity());
            }
        }

        if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH) {
            if (actor0->getScene().hasComponent<ScriptComponent>(actor0->getEntity())) {
                actor0->getScene().getComponent<ScriptComponent>(actor0->getEntity()).onCollisionExit(actor1->getEntity());
            }
            if (actor1->getScene().hasComponent<ScriptComponent>(actor1->getEntity())) {
                actor1->getScene().getComponent<ScriptComponent>(actor1->getEntity()).onCollisionExit(actor0->getEntity());
            }
        }
    }

    void PhysicsSimulationEventCallbacks::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
        for (uint32_t i = 0; i < count; i++) {
            if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
                continue;
            }

            auto* triggerActor = static_cast<AbstractPhysicsActor*>(pairs[i].triggerActor->userData);
            auto* otherActor = static_cast<AbstractPhysicsActor*>(pairs[i].otherActor->userData);

            if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                if (triggerActor->getScene().hasComponent<ScriptComponent>(triggerActor->getEntity())) {
                    triggerActor->getScene().getComponent<ScriptComponent>(triggerActor->getEntity()).onTriggerEnter(otherActor->getEntity());
                }
                if (otherActor->getScene().hasComponent<ScriptComponent>(otherActor->getEntity())) {
                    otherActor->getScene().getComponent<ScriptComponent>(otherActor->getEntity()).onTriggerEnter(triggerActor->getEntity());
                }
            }

            if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
                if (triggerActor->getScene().hasComponent<ScriptComponent>(triggerActor->getEntity())) {
                    triggerActor->getScene().getComponent<ScriptComponent>(triggerActor->getEntity()).onCollisionExit(otherActor->getEntity());
                }
                if (otherActor->getScene().hasComponent<ScriptComponent>(otherActor->getEntity())) {
                    otherActor->getScene().getComponent<ScriptComponent>(otherActor->getEntity()).onTriggerExit(triggerActor->getEntity());
                }
            }
        }
    }

    void PhysicsSimulationEventCallbacks::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}
}
