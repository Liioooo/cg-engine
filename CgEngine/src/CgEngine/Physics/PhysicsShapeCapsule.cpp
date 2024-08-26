#include "PhysicsShapeCapsule.h"
#include "PhysicsActor.h"
#include "Scene/Scene.h"
#include "PhysicsSystem.h"

namespace CgEngine {
    PhysicsShapeCapsule::PhysicsShapeCapsule(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, float radius, float halfHeight, glm::vec3 offset, bool isTrigger) : PhysicsShape() {
        auto scale = actor.getScene().getComponent<TransformComponent>(actor.getEntity()).getGlobalScale();
        float radiusScale = glm::max(scale.x, scale.z);

        physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radius * radiusScale, halfHeight * scale.y);
        physxShape = physx::PxRigidActorExt::createExclusiveShape(actor.getPhysxActor(), geometry, *physicsMaterial.getPhysxMaterial());
        physxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
        physxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

        physxShape->setSimulationFilterData(actor.getFilterData());

        physx::PxTransform transform(PhysXUtils::glmToPhysXVec(offset), PhysXUtils::glmToPhysXQuat(glm::quat(glm::vec3(0.0f, 0.0f, glm::half_pi<float>()))));
        physxShape->setLocalPose(transform);

        physxShape->userData = this;
    }
}
