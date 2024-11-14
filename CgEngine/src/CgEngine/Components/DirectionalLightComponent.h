#pragma once

#include "Component.h"

namespace CgEngine {

    struct DirectionalLightComponentParams {
        glm::vec3 color = {1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
        bool castShadows = true;

        void verifyParams() const;
    };

    class DirectionalLightComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, DirectionalLightComponentParams& params);
        void onRenderImGui() override;

        const glm::vec3& getColor() const;
        void setColor(const glm::vec3& value);
        float getIntensity() const;
        void setIntensity(float value);
        bool getCastShadows() const;
        void setCastShadows(bool value);

    private:
        glm::vec3 color;
        float intensity;
        bool castShadows;
    };

}
