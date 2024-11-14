#include "MeshRendererComponent.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void MeshRendererComponentParams::verifyParams() const {
        CG_ASSERT(customMesh != nullptr || !assetFile.empty() || !mesh.empty(), "MeshRendererComponentParams: 'customMesh', 'assetFile' or 'mesh' is required.")

        if (customMesh != nullptr) {
            for (uint32_t i = 0; i < customMesh->getMaterialCount(); i++) {
                CG_ASSERT(dynamic_cast<const PBRMaterial*>(customMesh->getMaterial(i)) != nullptr, "MeshRendererComponentParams: Your custom mesh does not only contain PBRMaterials!")
            }
            CG_ASSERT(!customMesh->getVAO()->getVertexBuffers().empty(), "CustomMesh does not have any Vertex data")
            CG_ASSERT(customMesh->getVAO()->getVertexBuffers()[0]->getLayout() == MeshProps::DEFAULT_VERT_BUFF_LAYOUT, "MeshRendererComponent only works with DEFAULT_VERT_BUFF_LAYOUT")
        }
    }

    void MeshRendererComponent::onAttach(Scene& scene, MeshRendererComponentParams &params) {
        auto& resourceManager = Application::get().getResourceManager();

        if (params.customMesh != nullptr) {
            customMesh = params.customMesh;
            for (uint32_t i = 0; i < customMesh->getMeshNodes().size(); i++) {
                if (!customMesh->getMeshNodes().at(i).submeshIndices.empty()) {
                    meshNodes.push_back(i);
                }
            }
        } else if (!params.mesh.empty()) {
            mesh = resourceManager.getResource<MeshVertices>(params.mesh);
            for (uint32_t i = 0; i < mesh->getMeshNodes().size(); i++) {
                if (!mesh->getMeshNodes().at(i).submeshIndices.empty()) {
                    meshNodes.push_back(i);
                }
            }
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
        enableCulling = params.enableCulling;
    }

    ResRef<MeshVertices> MeshRendererComponent::getMeshVertices() {
        return mesh;
    }

    CustomMesh* MeshRendererComponent::getCustomMesh() {
        return customMesh;
    }

    Mesh* MeshRendererComponent::getRenderMesh() {
        if (customMesh != nullptr) {
            return customMesh;
        }
        return mesh.get();
    }

    ResRef<PBRMaterial> MeshRendererComponent::getMaterial() {
        return material;
    }

    bool MeshRendererComponent::getCastShadows() const {
        return castShadows;
    }

    void MeshRendererComponent::setCastShadows(bool value) {
        castShadows = value;
    }

    bool MeshRendererComponent::getCullingEnabled() const {
        return enableCulling;
    }

    void MeshRendererComponent::setCullingEnabled(bool value) {
        enableCulling = value;
    }

    const std::vector<uint32_t>& MeshRendererComponent::getMeshNodes() {
        return meshNodes;
    }

    void MeshRendererComponent::setCustomMesh(CustomMesh* mesh) {
        for (uint32_t i = 0; i < mesh->getMaterialCount(); i++) {
            CG_ASSERT(dynamic_cast<const PBRMaterial*>(mesh->getMaterial(i)) != nullptr, "MeshRendererComponentParams: Your custom mesh does not only contain PBRMaterials!")
        }
        CG_ASSERT(!mesh->getVAO()->getVertexBuffers().empty(), "CustomMesh does not have any Vertex data")
        CG_ASSERT(mesh->getVAO()->getVertexBuffers()[0]->getLayout() == MeshProps::DEFAULT_VERT_BUFF_LAYOUT, "MeshRendererComponent only works with DEFAULT_VERT_BUFF_LAYOUT")

        customMesh = mesh;

        meshNodes.clear();
        for (uint32_t i = 0; i < customMesh->getMeshNodes().size(); i++) {
            if (!customMesh->getMeshNodes().at(i).submeshIndices.empty()) {
                meshNodes.push_back(i);
            }
        }
    }

    void MeshRendererComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("MeshRendererComponent")) {
        }
    }
}
