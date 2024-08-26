#include "PhysicsShapeSphere.h"
#include "PhysicsActor.h"
#include "Scene/Scene.h"
#include "PhysicsSystem.h"

namespace CgEngine {
    PhysicsShapeSphere::PhysicsShapeSphere(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, float radius, glm::vec3 offset, bool isTrigger) : PhysicsShape() {
        auto scale = actor.getScene().getComponent<TransformComponent>(actor.getEntity()).getGlobalScale();
        float largestScale = glm::max(scale.x, glm::max(scale.y, scale.z));

        physx::PxSphereGeometry geometry = physx::PxSphereGeometry(radius * largestScale);
        physxShape = physx::PxRigidActorExt::createExclusiveShape(actor.getPhysxActor(), geometry, *physicsMaterial.getPhysxMaterial());
        physxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
        physxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

        physxShape->setSimulationFilterData(actor.getFilterData());

        physx::PxTransform transform(PhysXUtils::glmToPhysXVec(offset));
        physxShape->setLocalPose(transform);

        physxShape->userData = this;
    }
}
