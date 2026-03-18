#include "RigidBodyComponent.h"
#include "Scene/Scene.h"
#include "imgui.h"
#include "Components/TransformComponent.h"
#include "Components/BoxColliderComponent.h"
#include "Components/SphereColliderComponent.h"
#include "Components/CapsuleColliderComponent.h"
#include "Components/TriangleColliderComponent.h"
#include "Components/ConvexColliderComponent.h"

namespace CgEngine {
    void RigidBodyComponentParams::verifyParams() const {}

    void RigidBodyComponent::onAttach(Scene& scene, RigidBodyComponentParams& params) {
        actor = new PhysicsActor(&scene, entity, params.isDynamic, params.collisionDetection);

        if (params.isDynamic) {
            setKinematic(params.isKinematic);
            setGravityDisabled(params.disableGravity);
            setMass(params.mass);
            setLinearDrag(params.linearDrag);
            setAngularDrag(params.angularDrag);
        }
    }

    void RigidBodyComponent::onEnable(Scene& scene) {
        wasEnabled = true;

        auto transform = scene.getComponent<TransformComponent>(entity);
        actor->getPhysxActor().setGlobalPose(physx::PxTransform(PhysXUtils::glmToPhysXVec(transform->getGlobalPosition()), PhysXUtils::glmToPhysXQuat(transform->getGlobalRotationQuat())));

        if (scene.hasComponent<BoxColliderComponent>(entity)) {
            auto colliderComp = scene.getComponent<BoxColliderComponent>(entity);
            if (!colliderComp->isColliderAddedToActor) {
                addBoxCollider(*colliderComp->getPhysicsMaterial().get(), colliderComp->getHalfSize(), colliderComp->getOffset(), colliderComp->getIsTrigger());
                colliderComp->isColliderAddedToActor = true;
            }
        }

        if (scene.hasComponent<SphereColliderComponent>(entity)) {
            auto colliderComp = scene.getComponent<SphereColliderComponent>(entity);
            if (!colliderComp->isColliderAddedToActor) {
                addSphereCollider(*colliderComp->getPhysicsMaterial().get(), colliderComp->getRadius(), colliderComp->getOffset(), colliderComp->getIsTrigger());
                colliderComp->isColliderAddedToActor = true;
            }
        }

        if (scene.hasComponent<CapsuleColliderComponent>(entity)) {
            auto colliderComp = scene.getComponent<CapsuleColliderComponent>(entity);
            if (!colliderComp->isColliderAddedToActor) {
                addCapsuleCollider(*colliderComp->getPhysicsMaterial().get(), colliderComp->getRadius(), colliderComp->getHalfHeight(), colliderComp->getOffset(), colliderComp->getIsTrigger());
                colliderComp->isColliderAddedToActor = true;
            }
        }

        if (scene.hasComponent<TriangleColliderComponent>(entity)) {
            auto colliderComp = scene.getComponent<TriangleColliderComponent>(entity);
            if (!colliderComp->isColliderAddedToActor) {
                addTriangleCollider(*colliderComp->getPhysicsMaterial().get(), colliderComp->getPhysicsMesh(), colliderComp->getIsTrigger());
                colliderComp->isColliderAddedToActor = true;
            }
        }

        if (scene.hasComponent<ConvexColliderComponent>(entity)) {
            auto colliderComp = scene.getComponent<ConvexColliderComponent>(entity);
            if (!colliderComp->isColliderAddedToActor) {
                addConvexCollider(*colliderComp->getPhysicsMaterial().get(), colliderComp->getPhysicsMesh(), colliderComp->getIsTrigger());
                colliderComp->isColliderAddedToActor = true;
            }
        }

        scene.getPhysicsScene().addActor(actor);

        for (auto& func : toCallAfterEnable) {
            func();
        }
        toCallAfterEnable.clear();
    }

    void RigidBodyComponent::onDetach(Scene& scene) {
        scene.getPhysicsScene().removeActor(actor);
        delete actor;

        if (scene.hasComponent<BoxColliderComponent>(entity)) {
            scene.getComponent<BoxColliderComponent>(entity)->isColliderAddedToActor = false;
        }

        if (scene.hasComponent<SphereColliderComponent>(entity)) {
            scene.getComponent<SphereColliderComponent>(entity)->isColliderAddedToActor = false;
        }

        if (scene.hasComponent<CapsuleColliderComponent>(entity)) {
            scene.getComponent<CapsuleColliderComponent>(entity)->isColliderAddedToActor = false;
        }

        if (scene.hasComponent<TriangleColliderComponent>(entity)) {
            scene.getComponent<TriangleColliderComponent>(entity)->isColliderAddedToActor = false;
        }

        if (scene.hasComponent<ConvexColliderComponent>(entity)) {
            scene.getComponent<ConvexColliderComponent>(entity)->isColliderAddedToActor = false;
        }
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
        if (!wasEnabled) {
            toCallAfterEnable.emplace_back([this, force, forceMode] {
                actor->addForce(force, forceMode);
            });
            return;
        }

        actor->addForce(force, forceMode);
    }

    void RigidBodyComponent::addTorque(glm::vec3 force, PhysicsForceMode forceMode) {
        if (!wasEnabled) {
            toCallAfterEnable.emplace_back([this, force, forceMode] {
                actor->addTorque(force, forceMode);
            });
            return;
        }

        actor->addTorque(force, forceMode);
    }

    void RigidBodyComponent::setKinematicTarget(glm::vec3 target, glm::quat rotation) {
        if (!wasEnabled) {
            toCallAfterEnable.emplace_back([this, target, rotation] {
                actor->setKinematicTarget(target, rotation);
            });
            return;
        }

        actor->setKinematicTarget(target, rotation);
    }

    void RigidBodyComponent::setGlobalPose(glm::vec3 target, glm::quat rotation) {
        actor->setGlobalPose(target, rotation);
    }

    glm::vec3 RigidBodyComponent::getGlobalPosePosition() {
        return actor->getGlobalPosePosition();
    }

    glm::vec3 RigidBodyComponent::getLinearVelocity() const {
        return actor->getLinearVelocity();
    }

    void RigidBodyComponent::setMaxLinearVelocity(float velocity) {
        actor->setMaxLinearVelocity(velocity);
    }

    void RigidBodyComponent::setMaxAngularVelocity(float velocity) {
        actor->setMaxAngularVelocity(velocity);
    }

    void RigidBodyComponent::addBoxCollider(PhysicsMaterial& material, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger) {
        actor->addBoxCollider(material, halfSize, offset, isTrigger);
    }

    void RigidBodyComponent::addSphereCollider(PhysicsMaterial& material, float radius, glm::vec3 offset, bool isTrigger) {
        actor->addSphereCollider(material, radius, offset, isTrigger);
    }

    void RigidBodyComponent::addCapsuleCollider(PhysicsMaterial& material, float radius, float halfHeight, glm::vec3 offset, bool isTrigger) {
        actor->addCapsuleCollider(material, radius, halfHeight, offset, isTrigger);
    }

    void RigidBodyComponent::addTriangleCollider(PhysicsMaterial& material, PhysicsTriangleMesh& physicsMesh, bool isTrigger) {
        actor->addTriangleCollider(material, physicsMesh, isTrigger);
    }

    void RigidBodyComponent::addConvexCollider(PhysicsMaterial& material, PhysicsConvexMesh& physicsMesh, bool isTrigger) {
        actor->addConvexCollider(material, physicsMesh, isTrigger);
    }

    void RigidBodyComponent::removeCollider(PhysicsColliderType colliderType) {
        actor->removeCollider(colliderType);
    }

    void RigidBodyComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("RigidBodyComponent")) {
        }
    }
}
