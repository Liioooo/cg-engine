#pragma once

#include "PxPhysicsAPI.h"

namespace CgEngine {

    class MeshVertices;

    class PhysicsTriangleMesh {
    public:
        explicit PhysicsTriangleMesh(physx::PxTriangleMesh* mesh) : mesh(mesh) {};

        physx::PxTriangleMesh* getPhysxMesh();

        MeshVertices& getVisualizationMesh();

    private:
        physx::PxTriangleMesh* mesh;
        MeshVertices* visualizationMesh = nullptr;
    };

}
