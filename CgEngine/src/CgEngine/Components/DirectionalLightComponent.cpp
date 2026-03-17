#include "DirectionalLightComponent.h"
#include "imgui.h"
#include "ImGui/ImGuiWidgets.h"
#include "CgEngineSharedUtils/LoaderUtils.h"
#include "CgEngineSharedUtils/StringUtils.h"

namespace CgEngine {
    void DirectionalLightComponentParams::verifyParams() const {}

    void DirectionalLightComponent::onAttach(Scene &scene, DirectionalLightComponentParams &params) {
        color = params.color;
        intensity = params.intensity;
        castShadows = params.castShadows;
    }

    const glm::vec3& DirectionalLightComponent::getColor() const {
        return color;
    }

    void DirectionalLightComponent::setColor(const glm::vec3& value) {
        color = value;
    }

    float DirectionalLightComponent::getIntensity() const {
        return intensity;
    }

    void DirectionalLightComponent::setIntensity(float value) {
        intensity = value;
    }

    bool DirectionalLightComponent::getCastShadows() const {
        return castShadows;
    }

    void DirectionalLightComponent::setCastShadows(bool value) {
        castShadows = value;
    }

    void DirectionalLightComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("DirectionalLightComponent")) {
            ImGui::ColorEdit3("Color", glm::value_ptr(color), ImGuiColorEditFlags_DisplayRGB);
            ImGui::DragFloat("Intesity", &intensity, 0.001, 0.0f, 1.0f);
            ImGui::Checkbox("Cast Shadows", &castShadows);
            ImGuiWidgets::copyCurrentConfig("DirectionalLightComponent", [this] (auto& map) {
                map["color"] = LoaderUtils::vec3ColorToHexString(color);
                map["intensity"] = std::to_string(intensity);
                map["cast-shadows"] = StringUtils::fromBool(castShadows);
            });
        }
    }

}
