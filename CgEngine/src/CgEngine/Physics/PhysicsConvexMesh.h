#pragma once

#include "PxPhysicsAPI.h"

namespace CgEngine {

    class MeshVertices;

    class PhysicsConvexMesh {
    public:
        explicit PhysicsConvexMesh(physx::PxConvexMesh* mesh) : mesh(mesh) {};

        physx::PxConvexMesh* getPhysxMesh();

        MeshVertices& getVisualizationMesh();

    private:
        physx::PxConvexMesh* mesh;
        MeshVertices* visualizationMesh = nullptr;
    };

}
