#include "PhysicsTriangleMesh.h"
#include "Resources/MeshVertices.h"

namespace CgEngine {
    physx::PxTriangleMesh* PhysicsTriangleMesh::getPhysxMesh() {
        return mesh;
    }

    MeshVertices& PhysicsTriangleMesh::getVisualizationMesh() {
        if (visualizationMesh != nullptr) {
            return *visualizationMesh;
        }

        visualizationMesh = MeshVertices::createFromPhysx((glm::vec3*)(mesh->getVertices()), mesh->getNbVertices(), (uint32_t*)mesh->getTriangles(), mesh->getNbTriangles() * 3);
        return *visualizationMesh;
    }
}