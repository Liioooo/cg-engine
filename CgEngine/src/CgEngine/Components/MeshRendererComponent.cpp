#include "MeshRendererComponent.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void MeshRendererComponent::onAttach(Scene& scene, MeshRendererComponentParams &params) {
        if (!params.mesh.empty()) {
            mesh = GlobalObjectManager::getInstance().getResourceManager().getResource<MeshVertices>(params.mesh);
            meshNodes.push_back(0);
        } else {
            mesh = GlobalObjectManager::getInstance().getResourceManager().getResource<MeshVertices>(params.assetFile);
            if (params.meshNodes.empty()) {
                for (uint32_t i = 0; i < mesh->getMeshNodes().size(); i++) {
                    if (!mesh->getMeshNodes().at(i).submeshIndices.empty()) {
                        meshNodes.push_back(i);
                    }
                }
            } else {
                for (const auto& nodeName: params.meshNodes) {
                    uint32_t i = mesh->getMeshNodeIndex(nodeName);
                    auto& mN = mesh->getMeshNodes().at(i);
                    if (!mN.submeshIndices.empty()) {
                        meshNodes.push_back(i);
                    }
                }
            }
        }

        if (params.material.empty()) {
            material = nullptr;
        } else {
            material = GlobalObjectManager::getInstance().getResourceManager().getResource<Material>(params.material);
        }

        castShadows = params.castShadows;
    }

    MeshVertices& MeshRendererComponent::getMeshVertices() {
        return *mesh;
    }

    Material* MeshRendererComponent::getMaterial() {
        return material;
    }

    bool MeshRendererComponent::getCastShadows() const {
        return castShadows;
    }

    void MeshRendererComponent::setCastShadows(bool value) {
        castShadows = value;
    }

    const std::vector<uint32_t>& MeshRendererComponent::getMeshNodes() {
        return meshNodes;
    }
}
