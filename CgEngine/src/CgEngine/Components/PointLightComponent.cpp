#include "PointLightComponent.h"
#include "imgui.h"

namespace CgEngine {
    void PointLightComponentParams::verifyParams() const {}

    void PointLightComponent::onAttach(Scene &scene, PointLightComponentParams &params) {
        color = params.color;
        intensity = params.intensity;
        radius = params.radius;
        falloff = params.falloff;
    }

    const glm::vec3& PointLightComponent::getColor() const {
        return color;
    }

    void PointLightComponent::setColor(const glm::vec3& value) {
        color = value;
    }

    float PointLightComponent::getIntensity() const {
        return intensity;
    }

    void PointLightComponent::setIntensity(float value) {
        intensity = value;
    }

    float PointLightComponent::getRadius() const {
        return radius;
    }

    void PointLightComponent::setRadius(float value) {
        radius = value;
    }

    float PointLightComponent::getFalloff() const {
        return falloff;
    }

    void PointLightComponent::setFalloff(float value) {
        falloff = value;
    }

    void PointLightComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("PointLightComponent")) {
            ImGui::ColorEdit3("Color", glm::value_ptr(color), ImGuiColorEditFlags_DisplayRGB);
            ImGui::DragFloat("Intesity", &intensity, 0.001, 0.0f, 1.0f);
            ImGui::DragFloat("Radius", &radius, 0.01, 0.0f);
            ImGui::DragFloat("Falloff", &falloff, 0.01, 0.0f);
        }
    }
}
