#include <Asserts.h>

#include <utility>
#include "CustomMesh.h"

namespace CgEngine {

    CustomMesh::CustomMesh() {
        vao = new VertexArrayObject();

        Submesh& submesh = submeshes.emplace_back();
        submesh.baseVertex = 0;
        submesh.baseIndex = 0;
        submesh.materialIndex = 0;

        MeshNode& meshNode = meshNodes.emplace_back();
        meshNode.aiNode = nullptr;
        meshNode.submeshIndices.push_back(0);
        meshNode.transform = glm::mat4(1.0f);
        meshNode.localTransform = glm::mat4(1.0f);
    }

    const Material* CustomMesh::getMaterial(size_t index) const {
        return material;
    }

    const uint32_t CustomMesh::getMaterialCount() const {
        return material != nullptr ? 1 : 0;
    }

    void CustomMesh::setMaterial(Material* material) {
        this->material = material;
    }

    AABoundingBox& CustomMesh::getBoundingBox()  {
        return meshNodes[0].aaBoundingBox;
    }
}
