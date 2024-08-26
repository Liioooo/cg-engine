#include "BoxColliderComponent.h"
#include "Scene/Scene.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void BoxColliderComponent::onAttach(Scene& scene, BoxColliderComponentParams& params) {
        halfSize = params.halfSize;
        offset = params.offset;
        isTrigger = params.isTrigger;

        physicsMaterial = GlobalObjectManager::getInstance().getResourceManager().getResource<PhysicsMaterial>(params.material);


        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            colliderUuid = scene.getComponent<RigidBodyComponent>(entity).addBoxCollider(*physicsMaterial, halfSize, offset, isTrigger);
        }
    }

    void BoxColliderComponent::onDetach(Scene& scene) {
        if (scene.hasComponent<RigidBodyComponent>(entity)) {
            scene.getComponent<RigidBodyComponent>(entity).removeCollider(colliderUuid);
        }
    }

    PhysicsMaterial& BoxColliderComponent::getPhysicsMaterial() {
        return *physicsMaterial;
    }

    glm::vec3 BoxColliderComponent::getHalfSize() const {
        return halfSize;
    }

    glm::vec3 BoxColliderComponent::getOffset() const {
        return offset;
    }

    bool BoxColliderComponent::getIsTrigger() const {
        return isTrigger;
    }
}
