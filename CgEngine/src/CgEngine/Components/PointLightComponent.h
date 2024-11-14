#pragma once

#include "Component.h"

namespace CgEngine {

    struct PointLightComponentParams {
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
        float radius = 5.0f;
        float falloff = 1.0f;

        void verifyParams() const;
    };

    class PointLightComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, PointLightComponentParams& params);
        void onRenderImGui() override;

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
