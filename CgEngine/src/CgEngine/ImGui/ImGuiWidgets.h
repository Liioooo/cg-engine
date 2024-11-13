#pragma once

#include "Application.h"
#include "Rendering/SceneRenderer.h"

namespace CgEngine::ImGuiWidgets {
    void applicationOptions(ApplicationOptions& applicationOptions);
    void performanceStats(float ts, const RenderingStats& renderingStats);
    void shaders(ShaderMap& shaderMap, ResourceManager& resourceManager);
}
