#pragma once

#include "Mesh.h"

namespace CgEngine {

    class CustomMesh : public Mesh {
    public:
        explicit CustomMesh();

        const Material* getMaterial(size_t index) const override;
        const uint32_t getMaterialCount() const override;

        void setMaterial(Material* material);
        void setBoundingBox(glm::vec3 max, glm::vec3 min);

        template<typename V>
        void setVertexData(const std::vector<V>& vertices, const std::vector<uint32_t>& indices, const std::vector<VertexBufferElement>& vertexBufferLayout) {
            if (!vao->getVertexBuffers().empty()) {
                vao->getVertexBuffers()[0]->setData(vertices.data(), vertices.size() * sizeof(V), VertexBufferUsage::Static);
            } else {
                auto* vertexBuffer = new VertexBuffer(vertices.data(), vertices.size() * sizeof(V), VertexBufferUsage::Static);
                vertexBuffer->setLayout(vertexBufferLayout);
                vao->addVertexBuffer(vertexBuffer);
            }

            vao->setIndexBuffer(indices.data(), indices.size());

            submeshes[0].vertexCount = vertices.size();
            submeshes[0].indexCount = indices.size();
        }

    private:
        Material* material = nullptr;
    };

}
