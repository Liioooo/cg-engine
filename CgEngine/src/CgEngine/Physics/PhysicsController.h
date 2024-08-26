#pragma once

#include "Scene/Entity.h"
#include "PxPhysicsAPI.h"
#include "AbstractPhysicsActor.h"

namespace CgEngine {

    class Scene;

    class PhysicsController : public AbstractPhysicsActor {
    public:
        PhysicsController(physx::PxController* physXController, bool hasGravity, glm::vec3 gravity, float stepDownOffset, Entity entity, Scene& scene);
        ~PhysicsController();

        PhysicsActorType getPhysicsActorType() const override;

        void update(float ts);
        void updateTransforms();

        void move(glm::vec3 dir);
        void setPosition(glm::vec3 pos);
        void jump(float strength);

        bool isGrounded() const;

    private:
        physx::PxController* physXController;
        bool hasGravity;
        float stepDownOffset;
        Entity entity;

        glm::vec3 movement = glm::vec3(0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        float nextJumpStrength = 0.0f;
        bool isJumping = false;
        glm::vec3 gravity;
        bool grounded = false;

        glm::vec3 desiredPosition;
        bool shouldTeleport = false;

        void updateIsGrounded();
        float getHeightOverGround();
    };

}
