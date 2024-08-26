#include "PhysicsShapeConvexMesh.h"
#include "PhysicsActor.h"
#include "Scene/Scene.h"
#include "PhysicsSystem.h"

namespace CgEngine {
    PhysicsShapeConvexMesh::PhysicsShapeConvexMesh(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, PhysicsConvexMesh& physicsMesh, bool isTrigger) : PhysicsShape() {
        auto scale = actor.getScene().getComponent<TransformComponent>(actor.getEntity()).getGlobalScale();

        physx::PxConvexMeshGeometry geometry = physx::PxConvexMeshGeometry(physicsMesh.getPhysxMesh(), physx::PxMeshScale(PhysXUtils::glmToPhysXVec(scale)));
        physxShape = physx::PxRigidActorExt::createExclusiveShape(actor.getPhysxActor(), geometry, *physicsMaterial.getPhysxMaterial());
        physxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
        physxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

        physxShape->setSimulationFilterData(actor.getFilterData());

        physxShape->userData = this;
    }
}
