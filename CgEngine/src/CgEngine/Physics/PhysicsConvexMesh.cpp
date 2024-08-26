#include "PhysicsConvexMesh.h"
#include "Resources/MeshVertices.h"
#include "PhysicsSystem.h"
#include "Asserts.h"

namespace CgEngine {
    physx::PxConvexMesh* PhysicsConvexMesh::getPhysxMesh() {
        return mesh;
    }

    MeshVertices& PhysicsConvexMesh::getVisualizationMesh() {
        if (visualizationMesh != nullptr) {
            return *visualizationMesh;
        }

        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        auto* convexVertices = mesh->getVertices();
        auto* convexIndices = mesh->getIndexBuffer();

        uint32_t offset = 0;

        for (uint32_t i = 0; i < mesh->getNbPolygons(); i++) {
            physx::PxHullPolygon face;
            bool status = mesh->getPolygonData(i, face);
            CG_ASSERT(status, "Error create ConvexMesh Visualization Data")

            const uint8_t* faceIndices = convexIndices + face.mIndexBase;
            for (uint32_t j = 0; j < face.mNbVerts; j++) {
                vertices.push_back(PhysXUtils::phsXToGlmVec(convexVertices[faceIndices[j]]));
            }

            for (uint32_t j = 2; j < face.mNbVerts; j++) {
                indices.emplace_back(offset);
                indices.emplace_back(offset + j);
                indices.emplace_back(offset + j - 1);
            }

            offset += face.mNbVerts;
        }

        visualizationMesh = MeshVertices::createFromPhysx(vertices.data(), vertices.size(), indices.data(), indices.size());
        return *visualizationMesh;
    }
}