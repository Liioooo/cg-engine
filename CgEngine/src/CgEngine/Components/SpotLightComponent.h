#pragma once

#include "Component.h"

namespace CgEngine {

    struct SpotLightComponentParams {
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
        float innerAngle;
        float outerAngle;
    };

    class SpotLightComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, SpotLightComponentParams& params);

        const glm::vec3& getColor() const;
        void setColor(const glm::vec3& value);
        float getIntensity() const;
        void setIntensity(float value);
        float getRadius() const;
        void setRadius(float value);
        float getFalloff() const;
        void setFalloff(float value);
        float getInnerAngle() const;
        void setInnerAngle(float value);
        float getOuterAngle() const;
        void setOuterAngle(float value);

    private:
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
        float innerAngle;
        float outerAngle;
    };

}
