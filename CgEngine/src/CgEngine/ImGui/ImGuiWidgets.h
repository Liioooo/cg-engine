#pragma once

#include "Application.h"
#include "Rendering/SceneRenderer.h"
#include "imgui.h"

namespace CgEngine::ImGuiWidgets {
    void applicationOptions(ApplicationOptions& applicationOptions);
    void performanceStats(float ts, const RenderingStats& renderingStats);
    void shaders(ShaderMap& shaderMap, ResourceManager& resourceManager);

    template <typename T, typename Getter, typename Setter>
    void dragFloat(const char* label, T* t, Getter getter, Setter setter, float speed, float min = 0.0f, float max = 0.0f) {
        float current = (t->*getter)();
        float new_value = current;

        ImGui::DragFloat(label, &new_value, speed, min, max);

        if (current != new_value) {
            (t->*setter)(new_value);
        }
    }

    template <typename T, typename Getter, typename Setter>
    void inputFloat(const char* label, T* t, Getter getter, Setter setter) {
        float current = (t->*getter)();
        float new_value = current;

        ImGui::InputFloat(label, &new_value);

        if (current != new_value) {
            (t->*setter)(new_value);
        }
    }
}
