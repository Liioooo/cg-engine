#pragma once

#include "Mesh.h"

namespace CgEngine {

    class CustomMesh : public Mesh {
    public:
        explicit CustomMesh(const std::vector<VertexBufferElement>& vertexBufferLayout);
        ~CustomMesh() override;

        const Material* getMaterial(size_t index) const override;
        const uint32_t getMaterialCount() const override;

        void setMaterial(Material* material);
        AABoundingBox& getBoundingBox();

        void buildMeshData();

        template<typename V>
        void setVertexData(const std::vector<V>& vertices, std::vector<uint32_t> indices, uint32_t lodLevel = 0) {
            if (lodMeshes.find(lodLevel) == lodMeshes.end()) {
                lodMeshes[lodLevel] = LodMesh{std::move(indices), static_cast<uint32_t>(vertices.size()), sizeof(V), allocateVertexData(vertices.size() * sizeof(V), vertices.data())};
            } else {
                freeVertexData(lodMeshes[lodLevel]);

                lodMeshes[lodLevel].indices = std::move(indices);
                lodMeshes[lodLevel].vertexCount = static_cast<uint32_t>(vertices.size());
                lodMeshes[lodLevel].vertexSize = sizeof(V);
                lodMeshes[lodLevel].vertexData = allocateVertexData(vertices.size() * sizeof(V), vertices.data());
            }

            CG_ASSERT(sizeof(V) == lodMeshes.begin()->second.vertexSize, "sizeof(V) must be the same for all LODs!");
        }

    private:
        struct LodMesh {
            std::vector<uint32_t> indices;
            uint32_t vertexCount;
            size_t vertexSize;
            void* vertexData = nullptr;
        };

        void* allocateVertexData(size_t size, const void* data);
        void freeVertexData(LodMesh& lodMesh);

        AABoundingBox boundingBox;
        Material* material = nullptr;
        std::map<uint32_t, LodMesh> lodMeshes;
    };

}
