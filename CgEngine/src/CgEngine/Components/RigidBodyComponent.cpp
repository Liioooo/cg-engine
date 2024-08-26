#include "RigidBodyComponent.h"
#include "Scene/Scene.h"

namespace CgEngine {
    void RigidBodyComponent::onAttach(Scene& scene, RigidBodyComponentParams& params) {
        auto& transform = scene.getComponent<TransformComponent>(entity);

        actor = new PhysicsActor(&scene, entity, transform.getGlobalPosition(), transform.getGlobalRotationQuat(), params.isDynamic, params.collisionDetection);

        if (params.isDynamic) {
            setKinematic(params.isKinematic);
            setGravityDisabled(params.disableGravity);
            setMass(params.mass);
            setLinearDrag(params.linearDrag);
            setAngularDrag(params.angularDrag);
        }

        if (scene.hasComponent<BoxColliderComponent>(entity)) {
            auto& colliderComp = scene.getComponent<BoxColliderComponent>(entity);
            uint32_t colliderUuid = addBoxCollider(colliderComp.getPhysicsMaterial(), colliderComp.getHalfSize(), colliderComp.getOffset(), colliderComp.getIsTrigger());
            colliderComp.colliderUuid = colliderUuid;
        }

        if (scene.hasComponent<SphereColliderComponent>(entity)) {
            auto& colliderComp = scene.getComponent<SphereColliderComponent>(entity);
            uint32_t colliderUuid = addSphereCollider(colliderComp.getPhysicsMaterial(), colliderComp.getRadius(), colliderComp.getOffset(), colliderComp.getIsTrigger());
            colliderComp.colliderUuid = colliderUuid;
        }

        if (scene.hasComponent<CapsuleColliderComponent>(entity)) {
            auto& colliderComp = scene.getComponent<CapsuleColliderComponent>(entity);
            uint32_t colliderUuid = addCapsuleCollider(colliderComp.getPhysicsMaterial(), colliderComp.getRadius(), colliderComp.getHalfHeight(), colliderComp.getOffset(), colliderComp.getIsTrigger());
            colliderComp.colliderUuid = colliderUuid;
        }

        if (scene.hasComponent<TriangleColliderComponent>(entity)) {
            auto& colliderComp = scene.getComponent<TriangleColliderComponent>(entity);
            uint32_t colliderUuid = addTriangleCollider(colliderComp.getPhysicsMaterial(), colliderComp.getPhysicsMesh(), colliderComp.getIsTrigger());
            colliderComp.colliderUuid = colliderUuid;
        }

        if (scene.hasComponent<ConvexColliderComponent>(entity)) {
            auto& colliderComp = scene.getComponent<ConvexColliderComponent>(entity);
            uint32_t colliderUuid = addConvexCollider(colliderComp.getPhysicsMaterial(), colliderComp.getPhysicsMesh(), colliderComp.getIsTrigger());
            colliderComp.colliderUuid = colliderUuid;
        }

        scene.getPhysicsScene().addActor(actor);
    }

    void RigidBodyComponent::onDetach(Scene& scene) {
        scene.getPhysicsScene().removeActor(actor);
        delete actor;
    }

    void RigidBodyComponent::setKinematic(bool isKinematic) {
        actor->setKinematic(isKinematic);
    }

    void RigidBodyComponent::setMass(float mass) {
        actor->setMass(mass);
    }

    void RigidBodyComponent::setLinearDrag(float drag) {
        actor->setLinearDrag(drag);
    }

    void RigidBodyComponent::setAngularDrag(float drag) {
        actor->setAngularDrag(drag);
    }

    void RigidBodyComponent::setGravityDisabled(bool disabled) {
        actor->setGravityDisabled(disabled);
    }

    bool RigidBodyComponent::isDynamic() const {
        return actor->isDynamic();
    }

    bool RigidBodyComponent::isKinematic() const {
        return actor->isKinematic();
    }

    void RigidBodyComponent::addForce(glm::vec3 force, PhysicsForceMode forceMode) {
        actor->addForce(force, forceMode);
    }

    void RigidBodyComponent::addTorque(glm::vec3 force, PhysicsForceMode forceMode) {
        actor->addTorque(force, forceMode);
    }

    void RigidBodyComponent::setKinematicTarget(glm::vec3 target, glm::quat rotation) {
        actor->setKinematicTarget(target, rotation);
    }

    void RigidBodyComponent::setGlobalPose(glm::vec3 target, glm::quat rotation) {
        actor->setGlobalPose(target, rotation);
    }

    glm::vec3 RigidBodyComponent::getGlobalPosePosition() {
        return actor->getGlobalPosePosition();
    }

    void RigidBodyComponent::setMaxLinearVelocity(float velocity) {
        actor->setMaxLinearVelocity(velocity);
    }

    void RigidBodyComponent::setMaxAngularVelocity(float velocity) {
        actor->setMaxAngularVelocity(velocity);
    }

    uint32_t RigidBodyComponent::addBoxCollider(PhysicsMaterial& material, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger) {
        return actor->addBoxCollider(material, halfSize, offset, isTrigger);
    }

    uint32_t RigidBodyComponent::addSphereCollider(PhysicsMaterial& material, float radius, glm::vec3 offset, bool isTrigger) {
        return actor->addSphereCollider(material, radius, offset, isTrigger);
    }

    uint32_t RigidBodyComponent::addCapsuleCollider(PhysicsMaterial& material, float radius, float halfHeight, glm::vec3 offset, bool isTrigger) {
        return actor->addCapsuleCollider(material, radius, halfHeight, offset, isTrigger);
    }

    uint32_t RigidBodyComponent::addTriangleCollider(PhysicsMaterial& material, PhysicsTriangleMesh& physicsMesh, bool isTrigger) {
        return actor->addTriangleCollider(material, physicsMesh, isTrigger);
    }

    uint32_t RigidBodyComponent::addConvexCollider(PhysicsMaterial& material, PhysicsConvexMesh& physicsMesh, bool isTrigger) {
        return actor->addConvexCollider(material, physicsMesh, isTrigger);
    }

    void RigidBodyComponent::removeCollider(uint32_t colliderUuid) {
        actor->removeCollider(colliderUuid);
    }
}
