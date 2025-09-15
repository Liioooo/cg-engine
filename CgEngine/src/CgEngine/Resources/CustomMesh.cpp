#include <Asserts.h>

#include <utility>
#include <Rendering/GraphicsObjectsFactory.h>
#include "CustomMesh.h"

namespace CgEngine {

    CustomMesh::CustomMesh(const std::vector<VertexBufferElement>& vertexBufferLayout) {
        vao = GraphicsObjectsFactory::createVertexArrayObject();

        auto* vertexBuffer = GraphicsObjectsFactory::createVertexBuffer(VertexBufferUsage::Static);
        vertexBuffer->setLayout(vertexBufferLayout);

        vao->addVertexBuffer(vertexBuffer);
    }

    CustomMesh::~CustomMesh() {
        for (auto& [key, lodMesh]: lodMeshes) {
            freeVertexData(lodMesh);
        }
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
        return boundingBox;
    }

    void CustomMesh::buildMeshData() {
        CG_ASSERT(!lodMeshes.empty(), "Trying to build CustomMesh without any mesh data")

        submeshes.clear();
        meshNodes.clear();

        if (lodMeshes.size() == 1) {
            Submesh& submesh = submeshes.emplace_back();
            submesh.baseVertex = 0;
            submesh.baseIndex = 0;
            submesh.materialIndex = 0;
            submesh.vertexCount = lodMeshes[0].vertexCount;
            submesh.indexCount = lodMeshes[0].indices.size();

            MeshNode& meshNode = meshNodes.emplace_back();
            meshNode.aiNode = nullptr;
            meshNode.submeshIndices.push_back(0);
            meshNode.transform = glm::mat4(1.0f);
            meshNode.localTransform = glm::mat4(1.0f);
            meshNode.aaBoundingBox = boundingBox;

            vao->getVertexBuffer(0)->setData(lodMeshes[0].vertexData, lodMeshes[0].vertexCount * lodMeshes[0].vertexSize);
            vao->setIndexBuffer(GraphicsObjectsFactory::createIndexBuffer(lodMeshes[0].indices.data(), lodMeshes[0].indices.size()));
        } else {
            MeshNode& lodOverviewNode = meshNodes.emplace_back();
            lodOverviewNode.aiNode = nullptr;
            lodOverviewNode.transform = glm::mat4(1.0f);
            lodOverviewNode.localTransform = glm::mat4(1.0f);
            lodOverviewNode.aaBoundingBox = boundingBox;

            uint32_t submeshIndex = 0;
            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;

            for (const auto& [_, lodMesh]: lodMeshes) {
                MeshNode& meshNode = meshNodes.emplace_back();
                meshNode.aiNode = nullptr;
                meshNode.transform = glm::mat4(1.0f);
                meshNode.localTransform = glm::mat4(1.0f);
                meshNode.aaBoundingBox = boundingBox;
                meshNode.isLodNode = true;
                meshNode.submeshIndices.push_back(submeshIndex);

                meshNodes[0].lodMeshNodes.emplace_back(meshNodes.size() - 1);

                Submesh& submesh = submeshes.emplace_back();
                submesh.baseVertex = vertexCount;
                submesh.baseIndex = indexCount;
                submesh.materialIndex = 0;
                submesh.vertexCount = lodMesh.vertexCount;
                submesh.indexCount = lodMesh.indices.size();

                vertexCount += lodMesh.vertexCount;
                indexCount += lodMesh.indices.size();
                submeshIndex++;
            }

            void* vertexData = malloc(vertexCount * lodMeshes[0].vertexSize);
            auto* indexData = (uint32_t*)malloc(indexCount * sizeof(uint32_t));

            uint32_t vertexOffset = 0;
            uint32_t indexOffset = 0;

            for (const auto& [_, lodMesh]: lodMeshes) {
                memcpy((char*)vertexData + vertexOffset * lodMesh.vertexSize, lodMesh.vertexData, lodMesh.vertexCount * lodMesh.vertexSize);

                for (uint32_t i = 0; i < lodMesh.indices.size(); i++) {
                    indexData[indexOffset + i] = lodMesh.indices[i];
                }
                vertexOffset += lodMesh.vertexCount;
                indexOffset += lodMesh.indices.size();
            }

            vao->getVertexBuffer(0)->setData(vertexData, vertexCount * lodMeshes[0].vertexSize);
            vao->setIndexBuffer(GraphicsObjectsFactory::createIndexBuffer(indexData, indexCount));

            free(vertexData);
            free(indexData);
        }
    }

    void* CustomMesh::allocateVertexData(size_t size, const void* data) {
        void* allocatedBuffer = malloc(size);
        memcpy(allocatedBuffer, data, size);
        return allocatedBuffer;
    }

    void CustomMesh::freeVertexData(LodMesh& lodMesh) {
        free(lodMesh.vertexData);
        lodMesh.vertexData = nullptr;
        lodMesh.vertexCount = 0;
        lodMesh.vertexSize = 0;
    }
}
