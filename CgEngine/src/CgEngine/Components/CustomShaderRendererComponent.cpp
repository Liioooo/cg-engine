#include "CustomShaderRendererComponent.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void CustomShaderRendererComponentParams::verifyParams() const {
        CG_ASSERT(customMesh != nullptr || !assetFile.empty() || !mesh.empty(), "CustomShaderRendererComponentParams: 'customMesh', 'assetFile' or 'mesh' is required.")
    }

    void CustomShaderRendererComponent::onAttach(Scene& scene, CustomShaderRendererComponentParams &params) {
        auto& resourceManager = Application::get().getResourceManager();

        if (params.customMesh != nullptr) {
            customMesh = params.customMesh;
            for (uint32_t i = 0; i < customMesh->getMeshNodes().size(); i++) {
                auto& mN = customMesh->getMeshNodes().at(i);
                if ((!mN.submeshIndices.empty() || !mN.lodMeshNodes.empty()) && !mN.isLodNode) {
                    meshNodes.push_back(i);
                }
            }
        } else if (!params.mesh.empty()) {
            mesh = resourceManager.getResource<MeshVertices>(params.mesh);
            for (uint32_t i = 0; i < mesh->getMeshNodes().size(); i++) {
                auto& mN = mesh->getMeshNodes().at(i);
                if ((!mN.submeshIndices.empty() || !mN.lodMeshNodes.empty()) && !mN.isLodNode) {
                    meshNodes.push_back(i);
                }
            }
        } else {
            mesh = resourceManager.getResource<MeshVertices>(params.assetFile);
            if (params.meshNodes.empty()) {
                for (uint32_t i = 0; i < mesh->getMeshNodes().size(); i++) {
                    auto& mN = mesh->getMeshNodes().at(i);
                    if ((!mN.submeshIndices.empty() || !mN.lodMeshNodes.empty()) && !mN.isLodNode) {
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

        instanceCount = params.instanceCount;

        enableCulling = params.enableCulling;

        if (params.boundingMax != glm::vec3(0.0f) || params.boundingMin != glm::vec3(0.0f)) {
            boundingBox.addBoxCoordinates(params.boundingMax, params.boundingMin);
        }

        pipeline = resourceManager.getResource<CustomGraphicsPipeline>(params.pipeline);
        descriptorSet = params.descriptorSet;
    }

    ResRef<MeshVertices> CustomShaderRendererComponent::getMeshVertices() {
        return mesh;
    }

    CustomMesh* CustomShaderRendererComponent::getCustomMesh() {
        return customMesh;
    }

    Mesh* CustomShaderRendererComponent::getRenderMesh() {
        if (customMesh != nullptr) {
            return customMesh;
        }
        return mesh.get();
    }

    bool CustomShaderRendererComponent::getCullingEnabled() const {
        return enableCulling;
    }

    void CustomShaderRendererComponent::setCullingEnabled(bool value) {
        enableCulling = value;
    }

    const std::vector<uint32_t>& CustomShaderRendererComponent::getMeshNodes() {
        return meshNodes;
    }

    void CustomShaderRendererComponent::setInstanceCount(uint32_t value) {
        instanceCount = value;
    }

    uint32_t CustomShaderRendererComponent::getInstanceCount() const {
        return instanceCount;
    }

    void CustomShaderRendererComponent::setCustomMesh(CustomMesh* mesh) {
        customMesh = mesh;

        meshNodes.clear();
        for (uint32_t i = 0; i < customMesh->getMeshNodes().size(); i++) {
            if (!customMesh->getMeshNodes().at(i).submeshIndices.empty()) {
                meshNodes.push_back(i);
            }
        }
    }

    const AABoundingBox* CustomShaderRendererComponent::getBoundingBox() {
        if (boundingBox.hasCoords()) {
            return &boundingBox;
        }
        return nullptr;
    }

    void CustomShaderRendererComponent::addBoundingBoxCoordinates(glm::vec3 max, glm::vec3 min) {
        boundingBox.addBoxCoordinates(max, min);
    }

    void CustomShaderRendererComponent::setBoundingBoxCenterAndExtents(glm::vec3 center, glm::vec3 extents) {
        boundingBox.setCenterAndExtents(center, extents);
    }

    bool CustomShaderRendererComponent::isActive() const {
        return active;
    }

    void CustomShaderRendererComponent::setActive(bool a) {
        active = a;
    }

    ResRef<CustomGraphicsPipeline> CustomShaderRendererComponent::getPipeline() {
        return pipeline;
    }

    void CustomShaderRendererComponent::setDescriptorSet(DescriptorSet* set) {
        descriptorSet = set;
    }

    const DescriptorSet* CustomShaderRendererComponent::getDescriptorSet() const {
        return descriptorSet;
    }

    void CustomShaderRendererComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CustomShaderRendererComponent")) {
        }
    }
}
