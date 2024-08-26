#include "SpotLightComponent.h"

namespace CgEngine {
    void SpotLightComponent::onAttach(Scene &scene, SpotLightComponentParams &params) {
        color = params.color;
        intensity = params.intensity;
        radius = params.radius;
        falloff = params.falloff;
        innerAngle = params.innerAngle;
        outerAngle = params.outerAngle;
    }

    const glm::vec3& SpotLightComponent::getColor() const {
        return color;
    }

    void SpotLightComponent::setColor(const glm::vec3& value) {
        color = value;
    }

    float SpotLightComponent::getIntensity() const {
        return intensity;
    }

    void SpotLightComponent::setIntensity(float value) {
        intensity = value;
    }

    float SpotLightComponent::getRadius() const {
        return radius;
    }

    void SpotLightComponent::setRadius(float value) {
        radius = value;
    }

    float SpotLightComponent::getFalloff() const {
        return falloff;
    }

    void SpotLightComponent::setFalloff(float value) {
        falloff = value;
    }

    float SpotLightComponent::getInnerAngle() const {
        return innerAngle;
    }

    void SpotLightComponent::setInnerAngle(float value) {
        innerAngle = value;
    }

    float SpotLightComponent::getOuterAngle() const {
        return outerAngle;
    }

    void SpotLightComponent::setOuterAngle(float value) {
        outerAngle = value;
    }
}
