#include "PointLightComponent.h"

namespace CgEngine {
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
}
