#include "CustomShaderRendererComponent.h"
#include "Application.h"

namespace CgEngine {
    void CustomShaderRendererComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty() || !mesh.empty(), "MeshRendererComponentParams: 'assetFile' or 'mesh' is required.")
    }

    void CustomShaderRendererComponent::onAttach(Scene& scene, CustomShaderRendererComponentParams &params) {
        auto& resourceManager = Application::get().getResourceManager();

        if (!params.mesh.empty()) {
            mesh = resourceManager.getResource<MeshVertices>(params.mesh);
            meshNodes.push_back(0);
        } else {
            mesh = resourceManager.getResource<MeshVertices>(params.assetFile);
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
            material = resourceManager.getResource<PBRMaterial>(params.material);
        }

        castShadows = params.castShadows;
    }

    ResRef<MeshVertices> CustomShaderRendererComponent::getMeshVertices() {
        return mesh;
    }

    ResRef<PBRMaterial> CustomShaderRendererComponent::getMaterial() {
        return material;
    }

    bool CustomShaderRendererComponent::getCastShadows() const {
        return castShadows;
    }

    void CustomShaderRendererComponent::setCastShadows(bool value) {
        castShadows = value;
    }

    const std::vector<uint32_t>& CustomShaderRendererComponent::getMeshNodes() {
        return meshNodes;
    }
}
