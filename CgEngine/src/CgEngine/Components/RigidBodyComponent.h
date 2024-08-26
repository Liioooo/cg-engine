#pragma once

#include "Physics/PhysicsActor.h"
#include "Physics/PhysicsSystem.h"
#include "Component.h"

namespace CgEngine {

    struct RigidBodyComponentParams {
        bool isDynamic;
        bool isKinematic;
        bool disableGravity;
        float mass;
        float linearDrag;
        float angularDrag;
        PhysicsCollisionDetection collisionDetection;
    };

    class RigidBodyComponent : public Component {
        friend class BoxColliderComponent;
        friend class SphereColliderComponent;
        friend class CapsuleColliderComponent;
        friend class TriangleColliderComponent;
        friend class ConvexColliderComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, RigidBodyComponentParams& params);
        void onDetach(Scene& scene) override;

        void setKinematic(bool isKinematic);
        void setMass(float mass);
        void setAngularDrag(float drag);
        void setLinearDrag(float drag);
        void setGravityDisabled(bool disabled);

        bool isDynamic() const;
        bool isKinematic() const;

        void addForce(glm::vec3 force, PhysicsForceMode forceMode = PhysicsForceMode::Force);
        void addTorque(glm::vec3 force, PhysicsForceMode forceMode = PhysicsForceMode::Force);

        void setKinematicTarget(glm::vec3 target, glm::quat rotation);
        void setGlobalPose(glm::vec3 target, glm::quat rotation);
        glm::vec3 getGlobalPosePosition();

        void setMaxLinearVelocity(float velocity);
        void setMaxAngularVelocity(float velocity);

    private:
        PhysicsActor* actor;

        uint32_t addBoxCollider(PhysicsMaterial& material, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger);
        uint32_t addSphereCollider(PhysicsMaterial& material, float radius, glm::vec3 offset, bool isTrigger);
        uint32_t addCapsuleCollider(PhysicsMaterial& material, float radius, float halfHeight, glm::vec3 offset, bool isTrigger);
        uint32_t addTriangleCollider(PhysicsMaterial& material, PhysicsTriangleMesh& physicsMesh, bool isTrigger);
        uint32_t addConvexCollider(PhysicsMaterial& material, PhysicsConvexMesh& physicsMesh, bool isTrigger);

        void removeCollider(uint32_t colliderUuid);
    };

}
