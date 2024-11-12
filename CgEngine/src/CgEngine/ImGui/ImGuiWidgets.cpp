#include "ImGuiWidgets.h"
#include "imgui.h"


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
        ImGui::Text("Frametime/FPS: %.2fms/%i", ts * 1000.0f, static_cast<int>(1.0f / ts));
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

}
