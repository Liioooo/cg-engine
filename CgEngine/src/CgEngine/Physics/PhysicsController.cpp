#include "PhysicsController.h"
#include "Scene/Scene.h"
#include "PhysicsScene.h"
#include "Asserts.h"

namespace CgEngine {
    PhysicsController::PhysicsController(physx::PxController* physXController, bool hasGravity, glm::vec3 gravity, float stepDownOffset, Entity entity, Scene& scene)
    : AbstractPhysicsActor(&scene, entity), physXController(physXController), hasGravity(hasGravity), gravity(gravity), stepDownOffset(stepDownOffset), entity(entity) {
        physXController->setUserData(this);
        physXController->getActor()->userData = this;
    }

    PhysicsController::~PhysicsController() {
        physXController->release();
    }

    PhysicsActorType PhysicsController::getPhysicsActorType() const {
        return PhysicsActorType::Controller;
    }

    void PhysicsController::update(float ts) {

        if (shouldTeleport) {
            physXController->setPosition(PhysXUtils::glmToExtendedPhysXVec(desiredPosition));
            shouldTeleport = false;
        } else {
            physx::PxControllerFilters filters;

            if (grounded && velocity.y < 0.0f) {
                velocity.y = 0.0f;
                isJumping = false;
            }

            updateIsGrounded();

            if (nextJumpStrength != 0.0f) {
                velocity += glm::sqrt(nextJumpStrength * -3.0f * gravity);
                nextJumpStrength = 0.0f;
                grounded = false;
                isJumping = true;
            }

            if (glm::length(movement) != 0.0f || !grounded) {
                if (hasGravity) {
                    velocity += gravity * ts;
                    movement.y += velocity.y * ts;
                }

                physx::PxControllerCollisionFlags collisionFlags = physXController->move(PhysXUtils::glmToPhysXVec(movement), 0.001f, ts, filters);
                if (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) {
                    grounded = true;
                }

                if (!(collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) && !isJumping) {
                    float heightOverGround = getHeightOverGround();
                    if (heightOverGround <= stepDownOffset) {
                        physXController->move(PhysXUtils::glmToPhysXVec({0.0f, -heightOverGround, 0.0f}), 0.001f, ts, filters);
                        grounded = true;
                    }
                }
            }

        }

        movement = {0.0f, 0.0f, 0.0f};
    }

    void PhysicsController::updateTransforms() {
        auto& transformComp = getScene().getComponent<TransformComponent>(entity);
        auto pos = PhysXUtils::phsXExtendedToGlmVec(physXController->getPosition());

        if (getScene().hasComponent<BoxColliderComponent>(entity)) {
            pos -= getScene().getComponent<BoxColliderComponent>(entity).getOffset();
        }
        if (getScene().hasComponent<CapsuleColliderComponent>(entity)) {
            pos -= getScene().getComponent<CapsuleColliderComponent>(entity).getOffset();
        }

        transformComp._physicsUpdate(pos, transformComp.getGlobalRotationQuat());
    }

    void PhysicsController::move(glm::vec3 dir) {
        movement += dir;
    }

    void PhysicsController::setPosition(glm::vec3 pos) {
        desiredPosition = pos;
        shouldTeleport = true;
    }

    void PhysicsController::jump(float strength) {
        CG_ASSERT(hasGravity, "Cannot jump is gravity is disabled")

        nextJumpStrength = strength;
    }

    bool PhysicsController::isGrounded() const {
        return grounded;
    }

    void PhysicsController::updateIsGrounded() {
        float heightOverGround = getHeightOverGround();
        grounded = heightOverGround >= 0.0f && ((grounded && heightOverGround < 0.05f) || (heightOverGround < 0.001f));
    }

    float PhysicsController::getHeightOverGround() {
        auto* physicsScene = static_cast<PhysicsScene*>(physXController->getScene()->userData);
        auto raycastHit = physicsScene->raycast(PhysXUtils::phsXExtendedToGlmVec(physXController->getFootPosition()), glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)), std::numeric_limits<float>::max(), std::unordered_set<Entity>{});
        return raycastHit.hitFound ? raycastHit.hitDistance : -1.0f;
    }
}