#include "DirectionalLightComponent.h"

namespace CgEngine {
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


}
