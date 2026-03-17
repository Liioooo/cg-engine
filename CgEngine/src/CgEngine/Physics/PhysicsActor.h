#pragma once

#include "PxPhysicsAPI.h"
#include "Physics/PhysicsSystem.h"
#include "PhysicsShape.h"
#include "AbstractPhysicsActor.h"

namespace CgEngine {

    enum class PhysicsColliderType {
        Box,
        Sphere,
        Capsule,
        TriangleMesh,
        ConvexMesh
    };

    class PhysicsActor : public AbstractPhysicsActor {
    public:
        PhysicsActor(Scene* scene, Entity entity, bool isDynamic, PhysicsCollisionDetection collisionDetection);
        ~PhysicsActor();

        PhysicsActorType getPhysicsActorType() const override;

        physx::PxRigidActor& getPhysxActor() const;
        const physx::PxFilterData& getFilterData() const;

        bool isSleeping() const;

        void updateTransforms();

        void setKinematic(bool isKinematic);
        void setMass(float mass);
        void setAngularDrag(float drag);
        void setLinearDrag(float drag);
        void setGravityDisabled(bool disabled);

        bool isDynamic() const;
        bool isKinematic() const;

        void addForce(glm::vec3 force, PhysicsForceMode forceMode);
        void addTorque(glm::vec3 force, PhysicsForceMode forceMode);
        void setKinematicTarget(glm::vec3 target, glm::quat rotation);
        void setGlobalPose(glm::vec3 target, glm::quat rotation);

        glm::vec3 getGlobalPosePosition();
        glm::vec3 getLinearVelocity() const;

        void setMaxLinearVelocity(float velocity);
        void setMaxAngularVelocity(float velocity);

        void addBoxCollider(PhysicsMaterial& material, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger);
        void addSphereCollider(PhysicsMaterial& material, float radius, glm::vec3 offset, bool isTrigger);
        void addCapsuleCollider(PhysicsMaterial& material, float radius, float halfHeight, glm::vec3 offset, bool isTrigger);
        void addTriangleCollider(PhysicsMaterial& material, PhysicsTriangleMesh& physicsMesh, bool isTrigger);
        void addConvexCollider(PhysicsMaterial& material, PhysicsConvexMesh& physicsMesh, bool isTrigger);
        void removeCollider(PhysicsColliderType colliderType);

    private:
        physx::PxRigidActor* physxActor;
        physx::PxFilterData filterData;
        bool dynamic;
        bool kinematic = false;

        std::unordered_map<PhysicsColliderType, PhysicsShape*> colliders;
    };

}
