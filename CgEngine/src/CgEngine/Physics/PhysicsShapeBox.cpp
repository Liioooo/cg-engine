#include "PhysicsShapeBox.h"
#include "PhysicsActor.h"
#include "Scene/Scene.h"
#include "PhysicsSystem.h"

namespace CgEngine {
    PhysicsShapeBox::PhysicsShapeBox(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, glm::vec3 halfSize, glm::vec3 offset, bool isTrigger) : PhysicsShape() {
        auto scale = actor.getScene().getComponent<TransformComponent>(actor.getEntity()).getGlobalScale();
        glm::vec3 size = glm::abs(scale * halfSize);

        physx::PxBoxGeometry geometry = physx::PxBoxGeometry(size.x, size.y, size.z);
        physxShape = physx::PxRigidActorExt::createExclusiveShape(actor.getPhysxActor(), geometry, *physicsMaterial.getPhysxMaterial());
        physxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
        physxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

        physxShape->setSimulationFilterData(actor.getFilterData());

        physx::PxTransform transform(PhysXUtils::glmToPhysXVec(offset));
        physxShape->setLocalPose(transform);

        physxShape->userData = this;
    }
}
