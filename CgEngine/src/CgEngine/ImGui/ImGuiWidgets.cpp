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
        ImGui::Text("G Buffer: %.6fms", renderingStats.gBufferTimer);
        ImGui::Text("HBAO Deinterleave: %.6fms", renderingStats.hbaoDeinterleavingTimer);
        ImGui::Text("HBAO Compute: %.6fms", renderingStats.hbaoComputeTimer);
        ImGui::Text("HBAO Reinterleave: %.6fms", renderingStats.hbaoReinterleavingTimer);
        ImGui::Text("HBAO Blur: %.6fms", renderingStats.hbaoBlurTimer);
        ImGui::Text("PBR: %.6fms", renderingStats.pbrTimer);
        ImGui::Text("Custom Shader: %.6fms", renderingStats.customShaderTimer);
        ImGui::Text("Skybox: %.6fms", renderingStats.skyboxTimer);
        ImGui::Text("Bloom: %.6fms", renderingStats.bloomTimer);
        ImGui::Text("Screen: %.6fms", renderingStats.screenTimer);
        ImGui::Text("UI Canvas: %.6fms", renderingStats.uiCanvasTimer);
        ImGui::Text("UI 2D: %.6fms", renderingStats.ui2DTimer);
    }

    void copyCurrentConfig(const std::string& compName, const std::function<void(std::unordered_map<std::string, std::string>&)>& getConfigMap) {
        ImGui::PushID(compName.c_str());
        if (ImGui::Button("Copy Config")) {
            std::unordered_map<std::string, std::string> configMap;
            getConfigMap(configMap);

            pugi::xml_document doc;
            pugi::xml_node node = doc.append_child(compName.c_str());

            for (const auto& [key, val]: configMap) {
                node.append_attribute(key.c_str()).set_value(val.c_str());
            }

            std::ostringstream stream;
            doc.print(stream);
            Application::get().getWindow().setClipboardText(stream.str().c_str());
        }
        ImGui::PopID();
    }

}
