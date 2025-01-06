#include "LodDistanceComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void LodDistanceComponentParams::verifyParams() const {
        CG_ASSERT(!lodDistances.empty(), "LodDistanceComponentParams: lodDistances must have at least one element")
    }

    void LodDistanceComponent::onAttach(Scene& scene, LodDistanceComponentParams& params) {
        lodDistances = params.lodDistances;
    }

    const std::vector<float>& LodDistanceComponent::getLodDistances() const {
        return lodDistances;
    }

    void LodDistanceComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("LodDistanceComponent")) {
        }
    }
}
