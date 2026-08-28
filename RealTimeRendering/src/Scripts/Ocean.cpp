#include "Ocean.h"

#include "imgui.h"
#include "GPUDebugGroup.h"
#include "CgEngine/Rendering/GraphicsObjectsFactory.h"
#include "Components/CustomShaderRendererComponent.h"

namespace RTR {
    CgEngine::CustomMesh* Ocean::createPlane(glm::vec2 center, glm::vec2 size, int segments) {
        auto* _mesh = new CgEngine::CustomMesh(CgEngine::MeshProps::DEFAULT_VERT_BUFF_LAYOUT);
        std::vector<CgEngine::MeshProps::Vertex> vertices;
        std::vector<uint32_t> indices;

        float wh = size.x / 2.0f;
        float dh = size.y / 2.0f;

        float dx = 1.0f / segments;
        float dy = 1.0f / segments;

        for (int y = 0; y < segments + 1; ++y) {
            for (int x = 0; x < segments + 1; ++x) {
                vertices.emplace_back(
                    center.x -wh + x * dx * (2 * wh),
                    0,
                    center.y -dh + y * dy * (2 * dh),
                    0,
                    1,
                    0,
                    0 + x * dx,
                    0 + y * dy
                );
            }
        }

        for (int y = 0; y < segments; ++y) {
            for (int x = 0; x < segments; ++x) {
                indices.push_back(x + y * (segments + 1));
                indices.push_back(x + (y + 1) * (segments + 1));
                indices.push_back((x + 1) + y * (segments + 1));
                indices.push_back((x + 1) + (y + 1) * (segments + 1));
            }
        }

        _mesh->setVertexData(vertices, indices);
        _mesh->getBoundingBox().addBoxCoordinates({center.x -wh, -0.25f, center.y -dh}, {center.x + wh, 2.0f, center.y + dh});
        _mesh->buildMeshData();
        return _mesh;
    }

    void Ocean::createMesh() {
        mesh = createPlane({0, 0}, {1000, 1000}, 285);
    }

    void Ocean::reinitialise()
    {
        float boundary1 = 2 * glm::pi<float>() / oceanParams1.length * 6;
        float boundary2 = 2 * glm::pi<float>() / oceanParams2.length * 6;

        oceanParams0.spectrumParams.cutoffHigh = boundary1;
        oceanParams1.spectrumParams.cutoffLow = boundary1;
        oceanParams1.spectrumParams.cutoffHigh = boundary2;
        oceanParams2.spectrumParams.cutoffLow = boundary2;

        delete oceanCascade0;
        delete oceanCascade1;
        delete oceanCascade2;

        oceanCascade0 = new OceanCascade(oceanParams0, CgEngine::Application::get().getResourceManager());
        oceanCascade1 = new OceanCascade(oceanParams1, CgEngine::Application::get().getResourceManager());
        oceanCascade2 = new OceanCascade(oceanParams2, CgEngine::Application::get().getResourceManager());

        oceanCascade0->calculateInitialState();
        oceanCascade1->calculateInitialState();
        oceanCascade2->calculateInitialState();
        updateMaterial();
    }

    void Ocean::updateMaterial() {
        MaterialUniformBufferData matData{};
        matData.foamColor = materialParams.foamColor;
        matData.sssColor = materialParams.sssColor;
        matData.color = materialParams.color;
        matData.roughness = materialParams.roughness;
        matData.roughnessScale = materialParams.roughnessScale;
        matData.maxGloss = materialParams.maxGloss;
        matData.foamBias = materialParams.foamBias;
        matData.foamScale = materialParams.foamScale;
        matData.length0 = oceanParams0.length;
        matData.length1 = oceanParams1.length;
        matData.length2 = oceanParams2.length;

        matUniformBuffer->setData(&matData, sizeof(MaterialUniformBufferData));

        CgEngine::DescriptorSetSpecification matSpec{};
        matSpec.uboBindings = {
            {2, matUniformBuffer}
        };
        matSpec.attachmentTextureBindings = {
            {10, ~0u, true, oceanCascade0->displacement},
            {11, ~0u, true, oceanCascade0->derivatives},
            {12, ~0u, true, oceanCascade0->turbulence},
            {13, ~0u, true, oceanCascade1->displacement},
            {14, ~0u, true, oceanCascade1->derivatives},
            {15, ~0u, true, oceanCascade1->turbulence},
            {17, ~0u, true, oceanCascade2->displacement},
            {18, ~0u, true, oceanCascade2->derivatives},
            {19, ~0u, true, oceanCascade2->turbulence},
        };

        mat->reconfigure(matSpec);
    }

    void Ocean::onEnable() {
        materialParams = {
            glm::vec3(1.0),
            glm::vec3(0.1541919, 0.8857628, 0.990566),
            glm::vec3(0.03457636, 0.12297464, 0.1981132),
            0.311,
            0.0044,
            0.91,
            2.72,
            0.3,
        };

        oceanParams0 = {
            256,
            250,
            500,
            9.81,
            {
                30,
                3.3,
                -1, // Will be calculated
                -1, // Will be calculated
                0.0001f,
                9999,
                {0, 0} // Currently not used
            }
        };

        oceanParams1 = OceanParams(oceanParams0);
        oceanParams1.length = 27;
        oceanParams2 = OceanParams(oceanParams0);
        oceanParams2.length = 5;

        createMesh();

        const auto* layout = getResource<CgEngine::CustomGraphicsPipeline>("ocean/render")->getDescriptorSetLayout();

        mat = CgEngine::GraphicsObjectsFactory::createDescriptorSet(layout);
        matUniformBuffer = CgEngine::GraphicsObjectsFactory::createUniformBuffer(sizeof(MaterialUniformBufferData));

        CgEngine::CustomShaderRendererComponentParams params;
        params.pipeline = "ocean/render";
        params.descriptorSet = mat;
        params.customMesh = mesh;
        params.instanceCount = 1;
        params.enableCulling = true;

        getOwingEntity().attachComponent<CgEngine::CustomShaderRendererComponent>(params);
        reinitialise();
    }

    void Ocean::update(CgEngine::TimeStep ts) {

        const auto tsSeconds = ts.getSeconds();

        currentTime = currentTime + ts.getSeconds();
        executeOnRender([this, tsSeconds](CgEngine::SceneRenderer&) {
            CG_GPU_DEBUG_GROUP("Ocean::update")
            oceanCascade0->calculateStateAtTime(currentTime, tsSeconds);
            oceanCascade1->calculateStateAtTime(currentTime, tsSeconds);
            oceanCascade2->calculateStateAtTime(currentTime, tsSeconds);
        });
    }

    void Ocean::onRenderImGui() {
        bool changed = false;
        bool needRecalculation = false;

        ImGui::PushID("OceanCascade0");
        ImGui::SeparatorText("Cascade 0");
        needRecalculation |= ImGui::DragFloat("Length", &oceanParams0.length);
        needRecalculation |= ImGui::DragFloat("Depth", &oceanParams0.depth);
        needRecalculation |= ImGui::DragFloat("Gravity", &oceanParams0.g);
        needRecalculation |= ImGui::DragFloat("Gamma", &oceanParams0.spectrumParams.gamma, 0.1);
        ImGui::PopID();

        ImGui::PushID("OceanCascade1");
        ImGui::SeparatorText("Cascade 1");
        needRecalculation |= ImGui::DragFloat("Length", &oceanParams1.length);
        needRecalculation |= ImGui::DragFloat("Depth", &oceanParams1.depth);
        needRecalculation |= ImGui::DragFloat("Gravity", &oceanParams1.g);
        needRecalculation |= ImGui::DragFloat("Gamma", &oceanParams1.spectrumParams.gamma, 0.1);
        ImGui::PopID();

        ImGui::PushID("OceanCascade2");
        ImGui::SeparatorText("Cascade 2");
        needRecalculation |= ImGui::DragFloat("Length", &oceanParams2.length);
        needRecalculation |= ImGui::DragFloat("Depth", &oceanParams2.depth);
        needRecalculation |= ImGui::DragFloat("Gravity", &oceanParams2.g);
        needRecalculation |= ImGui::DragFloat("Gamma", &oceanParams2.spectrumParams.gamma, 0.1);
        ImGui::PopID();

        ImGui::SeparatorText("Material Settings");
        changed |= ImGui::ColorEdit3("Color", glm::value_ptr(materialParams.color), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::ColorEdit3("Subsurface Color", glm::value_ptr(materialParams.sssColor), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::ColorEdit3("Foam Color", glm::value_ptr(materialParams.foamColor), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::SliderFloat("Roughness", &materialParams.roughness, 0, 1);
        changed |= ImGui::SliderFloat("Roughness Scale", &materialParams.roughnessScale, 0, 0.5);
        changed |= ImGui::SliderFloat("Max Gloss", &materialParams.maxGloss, 0, 1);
        changed |= ImGui::DragFloat("Foam Bias", &materialParams.foamBias, 0.1);
        changed |= ImGui::DragFloat("Foam Scale", &materialParams.foamScale, 0.05);

        if (needRecalculation) {
            reinitialise();
        } else if (changed) {
            updateMaterial();
        }
    }

    void Ocean::onDetach() {
        delete mesh;
        delete mat;
    }
}
