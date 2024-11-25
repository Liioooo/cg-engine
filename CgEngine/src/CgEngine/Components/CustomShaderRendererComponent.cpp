#include "CustomShaderRendererComponent.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void CustomShaderRendererComponentParams::verifyParams() const {
        CG_ASSERT(customMesh != nullptr || !assetFile.empty() || !mesh.empty(), "CustomShaderRendererComponentParams: 'customMesh', 'assetFile' or 'mesh' is required.")
        CG_ASSERT(!shader.empty(), "CustomShaderRendererComponentParams: shader must be set")

        if (customMesh != nullptr) {
            CG_ASSERT(!customMesh->getVAO()->getVertexBuffers().empty(), "CustomMesh does not have any Vertex data")
        }
    }

    void CustomShaderRendererComponent::onAttach(Scene& scene, CustomShaderRendererComponentParams &params) {
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

        if (params.customMaterial != nullptr) {
            customMaterial = params.customMaterial;
        }

        if (params.material.empty()) {
            pbrMaterial = nullptr;
        } else {
            pbrMaterial = resourceManager.getResource<PBRMaterial>(params.material);
        }

        shader = resourceManager.getResource<CustomShader>(params.shader);

        instanceCount = params.instanceCount;
        renderPassOptions = params.renderPassOptions;

        enableCulling = params.enableCulling;
        boundingBox.addBoxCoordinates(params.boundingMax, params.boundingMin);
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

    ResRef<PBRMaterial> CustomShaderRendererComponent::getPBRMaterial() {
        return pbrMaterial;
    }

    Material* CustomShaderRendererComponent::getCustomMaterial() {
        return customMaterial;
    }

    Material* CustomShaderRendererComponent::getRenderMaterial() {
        if (customMaterial != nullptr) {
            return customMaterial;
        }
        return pbrMaterial.get();
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

    ResRef<CustomShader> CustomShaderRendererComponent::getShader() {
        return shader;
    }

    void CustomShaderRendererComponent::setCustomMesh(CustomMesh* mesh) {
        CG_ASSERT(!mesh->getVAO()->getVertexBuffers().empty(), "CustomMesh does not have any Vertex data")

        customMesh = mesh;

        meshNodes.clear();
        for (uint32_t i = 0; i < customMesh->getMeshNodes().size(); i++) {
            if (!customMesh->getMeshNodes().at(i).submeshIndices.empty()) {
                meshNodes.push_back(i);
            }
        }
    }

    void CustomShaderRendererComponent::setCustomMaterial(Material* material) {
        customMaterial = material;
    }

    CustomShaderRendererComponentRenderPassOptions& CustomShaderRendererComponent::getRenderPassOptions() {
        return renderPassOptions;
    }

    AABoundingBox& CustomShaderRendererComponent::getBoundingBox() {
        return boundingBox;
    }

    void CustomShaderRendererComponent::setInstanceBuffer1(ShaderStorageBuffer* instanceBuffer) {
        instanceBuffers.first = instanceBuffer;
    }

    void CustomShaderRendererComponent::setInstanceBuffer2(ShaderStorageBuffer* instanceBuffer) {
        instanceBuffers.second = instanceBuffer;
    }

    std::pair<ShaderStorageBuffer*, ShaderStorageBuffer*> CustomShaderRendererComponent::getInstanceBuffers() {
        return instanceBuffers;
    }

    bool CustomShaderRendererComponent::isActive() const {
        return active;
    }

    void CustomShaderRendererComponent::setActive(bool a) {
        active = a;
    }

    void CustomShaderRendererComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("CustomShaderRendererComponent")) {
        }
    }
}
