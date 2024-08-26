#include "CapsuleColliderComponent.h"
#include "Scene/Scene.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void CapsuleColliderComponent::onAttach(Scene& scene, CapsuleColliderComponentParams& params) {
        radius = params.radius;
        offset = params.offset;
        isTrigger = params.isTrigger;
        halfHeight = params.halfHeight;

        physicsMaterial = GlobalObjectManager::getInstance().getResourceManager().getResource<PhysicsMaterial>(params.material);


        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            colliderUuid = scene.getComponent<RigidBodyComponent>(entity).addCapsuleCollider(*physicsMaterial, radius, halfHeight, offset, isTrigger);
        }
    }

    void CapsuleColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity).removeCollider(colliderUuid);
        }
    }

    PhysicsMaterial& CapsuleColliderComponent::getPhysicsMaterial() {
        return *physicsMaterial;
    }

    float CapsuleColliderComponent::getRadius() const {
        return radius;
    }

    float CapsuleColliderComponent::getHalfHeight() const {
        return halfHeight;
    }

    glm::vec3 CapsuleColliderComponent::getOffset() const {
        return offset;
    }

    bool CapsuleColliderComponent::getIsTrigger() const {
        return isTrigger;
    }
}
