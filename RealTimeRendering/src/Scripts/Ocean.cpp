#include "Ocean.h"

#include "imgui.h"
#include "OpenGLDebugGroup.h"

namespace RTR {
    CgEngine::CustomMesh* Ocean::createPlane(glm::vec2 center, glm::vec2 size, int segments) {
        auto* _mesh = new CgEngine::CustomMesh(CgEngine::MeshProps::DEFAULT_VERT_BUFF_LAYOUT);
        std::vector<CgEngine::MeshProps::Vertex> vertices;
        std::vector<uint32_t> indices;

        float wh = size.x / 2.0;
        float dh = size.y / 2.0;

        float dx = 1.0 / segments;
        float dy = 1.0 / segments;

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

    void Ocean::updateMaterial()
    {
        mat->setTexture2D("u_displacementC0", *oceanCascade0->displacement, 10);
        mat->setTexture2D("u_derivativesC0", *oceanCascade0->derivatives, 11);
        mat->setTexture2D("u_turbulenceC0", *oceanCascade0->turbulence, 12);
        mat->setTexture2D("u_displacementC1", *oceanCascade1->displacement, 13);
        mat->setTexture2D("u_derivativesC1", *oceanCascade1->derivatives, 14);
        mat->setTexture2D("u_turbulenceC1", *oceanCascade1->turbulence, 15);
        mat->setTexture2D("u_displacementC2", *oceanCascade2->displacement, 16);
        mat->setTexture2D("u_derivativesC2", *oceanCascade2->derivatives, 17);
        mat->setTexture2D("u_turbulenceC2", *oceanCascade2->turbulence, 18);
        mat->set("u_length0", oceanParams0.length);
        mat->set("u_length1", oceanParams1.length);
        mat->set("u_length2", oceanParams2.length);
        mat->set("u_foamColor", materialParams.foamColor);
        mat->set("u_sssColor", materialParams.sssColor);
        mat->set("u_color", materialParams.color);
        mat->set("u_roughness", materialParams.roughness);
        mat->set("u_roughnessScale", materialParams.roughnessScale);
        mat->set("u_maxGloss", materialParams.maxGloss);
        mat->set("u_foamBias", materialParams.foamBias);
        mat->set("u_foamScale", materialParams.foamScale);
    }

    void Ocean::onAttach() {
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

        mat = new CgEngine::CustomValMaterial();
        createMesh();
        reinitialise();

        CgEngine::CustomShaderRendererComponentParams params;
        params.shader = "ocean/render";
        params.customMesh = mesh;
        params.instanceCount = 1;
        params.customMaterial = mat;
        params.enableCulling = true;
        params.renderPassOptions.tesselationPatchSize = 4;

        auto& c = attachComponent<CgEngine::CustomShaderRendererComponent>(params);

        onPreRenderCbUuid = addOnPreRenderCallback([](const CgEngine::CameraFrustum& camaraFrustum) {
            CG_LOGGING_DEBUG("On PreRender")
        }, true);
    }

    void Ocean::update(CgEngine::TimeStep ts) {
        currentTime = currentTime + ts.getSeconds();
        CG_GPU_DEBUG_GROUP("Ocean::update")
        oceanCascade0->calculateStateAtTime(currentTime, ts.getSeconds());
        oceanCascade1->calculateStateAtTime(currentTime, ts.getSeconds());
        oceanCascade2->calculateStateAtTime(currentTime, ts.getSeconds());
    }

    void Ocean::onKeyPressed(CgEngine::KeyPressedEvent& event) {
    }

    void Ocean::onRenderImGui() {
        bool changed = false;
        bool needRecaluclation = false;

        ImGui::PushID("OceanCascade0");
        ImGui::SeparatorText("Cascade 0");
        needRecaluclation |= ImGui::DragFloat("Length", &oceanParams0.length);
        needRecaluclation |= ImGui::DragFloat("Depth", &oceanParams0.depth);
        needRecaluclation |= ImGui::DragFloat("Gravity", &oceanParams0.g);
        needRecaluclation |= ImGui::DragFloat("Gamma", &oceanParams0.spectrumParams.gamma, 0.1);
        ImGui::PopID();

        ImGui::PushID("OceanCascade1");
        ImGui::SeparatorText("Cascade 1");
        needRecaluclation |= ImGui::DragFloat("Length", &oceanParams1.length);
        needRecaluclation |= ImGui::DragFloat("Depth", &oceanParams1.depth);
        needRecaluclation |= ImGui::DragFloat("Gravity", &oceanParams1.g);
        needRecaluclation |= ImGui::DragFloat("Gamma", &oceanParams1.spectrumParams.gamma, 0.1);
        ImGui::PopID();

        ImGui::PushID("OceanCascade2");
        ImGui::SeparatorText("Cascade 2");
        needRecaluclation |= ImGui::DragFloat("Length", &oceanParams2.length);
        needRecaluclation |= ImGui::DragFloat("Depth", &oceanParams2.depth);
        needRecaluclation |= ImGui::DragFloat("Gravity", &oceanParams2.g);
        needRecaluclation |= ImGui::DragFloat("Gamma", &oceanParams2.spectrumParams.gamma, 0.1);
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

        if (needRecaluclation) {
            reinitialise();
        } else if (changed) {
            updateMaterial();
        }
    }

    void Ocean::onDetach() {
        removeOnPreRenderCallback(onPreRenderCbUuid);

        delete mesh;
        delete instanceBuffer;
        delete mat;
    }
}
