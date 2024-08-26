#pragma once

#include "Component.h"

namespace CgEngine {

    struct PointLightComponentParams {
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
    };

    class PointLightComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, PointLightComponentParams& params);

        const glm::vec3& getColor() const;
        void setColor(const glm::vec3& value);
        float getIntensity() const;
        void setIntensity(float value);
        float getRadius() const;
        void setRadius(float value);
        float getFalloff() const;
        void setFalloff(float value);

    private:
        glm::vec3 color;
        float intensity;
        float radius;
        float falloff;
    };

}
