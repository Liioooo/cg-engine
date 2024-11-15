#include "ImGuiWidgets.h"
#include "imgui.h"
#include "Application.h"
#include "pugixml.hpp"

namespace CgEngine::ImGuiWidgets {
    void applicationOptions(ApplicationOptions& applicationOptions) {
        ImGui::Checkbox("Debug Show Physics Colliders", &applicationOptions.debugShowPhysicsColliders);
        ImGui::Checkbox("Debug Show Bounding Boxes", &applicationOptions.debugShowBoundingBoxes);
        ImGui::Checkbox("Debug Show Normals", &applicationOptions.debugShowNormals);
        ImGui::Checkbox("Debug Render Lines", &applicationOptions.debugRenderLines);
        ImGui::Checkbox("Enable Bloom", &applicationOptions.enableBloom);
        ImGui::Checkbox("Enable HBAO", &applicationOptions.enableHBAO);
    }

    void performanceStats(float ts, const RenderingStats& renderingStats) {
        ImGui::SeparatorText("Performance");
        ImGui::Text("Frametime/FPS: %.2fms/%i", ts * 1000.0f, static_cast<int>(1.0f / ts));
        ImGui::SeparatorText("GPU Timers");
        ImGui::Text("Skin Meshes: %.6fms", renderingStats.skinMeshesTimer);
        ImGui::Text("Shadow Maps: %.6fms", renderingStats.shadowMapTimer);
        ImGui::Text("Pre Depth: %.6fms", renderingStats.preDepthTimer);
        ImGui::Text("HBAO Deinterleave: %.6fms", renderingStats.hbaoDeinterleavingTimer);
        ImGui::Text("HBAO Compute: %.6fms", renderingStats.hbaoComputeTimer);
        ImGui::Text("HBAO Reinterleave: %.6fms", renderingStats.hbaoReinterleavingTimer);
        ImGui::Text("HBAO Blur: %.6fms", renderingStats.hbaoBlurTimer);
        ImGui::Text("Geometry: %.6fms", renderingStats.geometryTimer);
        ImGui::Text("Custom Shader: %.6fms", renderingStats.customShaderTimer);
        ImGui::Text("Skybox: %.6fms", renderingStats.skyboxTimer);
        ImGui::Text("Bloom: %.6fms", renderingStats.bloomTimer);
        ImGui::Text("Screen: %.6fms", renderingStats.screenTimer);
        ImGui::Text("UI: %.6fms", renderingStats.uiTimer);
    }

    void shaders(ShaderMap& shaderMap, ResourceManager& resourceManager) {
        int id = 0;

        ImGui::SeparatorText("Engine");

        ImGui::Text("dirShadowMap");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.dirShadowMapShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("preDepth");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.preDepthShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("hbaoDeinterleaving");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.hbaoDeinterleavingShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("hbao");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.hbaoShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("hbaoReinterleaving");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.hbaoReinterleavingShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("hbaoBlur");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.hbaoBlurShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("pbr");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.geometryShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("skybox");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.skyboxShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("bloomDownSample");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.bloomDownSampleShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("bloomUpSample");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.bloomUpSampleShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("physicsCollider (colliders)");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.physicsCollidersShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("boundingBox (colliders)");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.boundingBoxShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("normalsVisualize");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.normalsDebugShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("lines");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.debugLinesShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("screen");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.screenShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("uiCircle");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.uiCircleShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("uiRect");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.uiRectShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("uiText");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.uiTextShader->reload();
        }
        ImGui::PopID();

        ImGui::Text("skinning");
        ImGui::SameLine();
        ImGui::PushID(id++);
        if (ImGui::SmallButton("Reload")) {
            shaderMap.skinningShader->reload();
        }
        ImGui::PopID();

        ImGui::SeparatorText("Custom");

        for (auto it = resourceManager.begin<CustomShader>(); it != resourceManager.end<CustomShader>(); it++) {
            ImGui::Text("%s", it->first.c_str());
            ImGui::SameLine();
            ImGui::PushID(id++);
            if (ImGui::SmallButton("Reload")) {
                it->second->reload();
            }
            ImGui::PopID();
        }

        ImGui::SeparatorText("Custom Compute");

        for (auto it = resourceManager.begin<CustomComputeShader>(); it != resourceManager.end<CustomComputeShader>(); it++) {
            ImGui::Text("%s", it->first.c_str());
            ImGui::SameLine();
            ImGui::PushID(id++);
            if (ImGui::SmallButton("Reload")) {
                it->second->reload();
            }
            ImGui::PopID();
        }
    }

    void ImGuiWidgets::copyCurrentConfig(const std::string& compName, const std::function<std::unordered_map<std::string, std::string>()>& getConfigMap) {
        if (ImGui::Button("Copy Config")) {
            const auto& configMap = getConfigMap();

            pugi::xml_document doc;
            pugi::xml_node node = doc.append_child(compName.c_str());

            for (const auto& [key, val]: configMap) {
                node.append_attribute(key.c_str()).set_value(val.c_str());
            }

            std::ostringstream stream;
            doc.print(stream);
            Application::get().getWindow().setClipboardText(stream.str().c_str());
        }
    }

}
