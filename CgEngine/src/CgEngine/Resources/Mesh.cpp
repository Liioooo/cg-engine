#include "Mesh.h"

namespace CgEngine {
    Mesh::~Mesh() {
        delete vao;
    }

    VertexArrayObject* Mesh::getVAO() {
        return vao;
    }

    const std::vector<Submesh>& Mesh::getSubmeshes() const {
        return submeshes;
    }

    std::vector<MeshNode>& Mesh::getMeshNodes() {
        return meshNodes;
    }

    uint32_t Mesh::getMeshNodeIndex(const std::string& nodeName) const {
        return nodeNameToNode.at(nodeName);
    }
}
