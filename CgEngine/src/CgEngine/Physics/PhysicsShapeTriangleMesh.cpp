#include "PhysicsShapeTriangleMesh.h"
#include "PhysicsActor.h"
#include "Scene/Scene.h"
#include "PhysicsSystem.h"

namespace CgEngine {
    PhysicsShapeTriangleMesh::PhysicsShapeTriangleMesh(PhysicsActor& actor, PhysicsMaterial& physicsMaterial, PhysicsTriangleMesh& physicsMesh, bool isTrigger) : PhysicsShape() {
        auto scale = actor.getScene().getComponent<TransformComponent>(actor.getEntity()).getGlobalScale();

        physx::PxTriangleMeshGeometry geometry = physx::PxTriangleMeshGeometry(physicsMesh.getPhysxMesh(), physx::PxMeshScale(PhysXUtils::glmToPhysXVec(scale)));
        physxShape = physx::PxRigidActorExt::createExclusiveShape(actor.getPhysxActor(), geometry, *physicsMaterial.getPhysxMaterial());
        physxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
        physxShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

        physxShape->setSimulationFilterData(actor.getFilterData());

        physxShape->userData = this;
    }
}
