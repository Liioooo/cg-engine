#pragma once

#include "Component.h"
#include "Rendering/Camera.h"

namespace CgEngine {

    struct CameraComponentParams {
        std::string projection = "perspective";
        float cnear = 0.1f;
        float cfar = 100.0f;
        float cfov = 60.0f;
        float orthoSize = 10.0f;
        bool isPrimary = false;
        float exposure = 1.0f;
        float bloomIntensity = 1.0f;
        float bloomThreshold = 0.2f;

        float hbaoRadius = 1.0f;
        float hbaoIntensity = 1.5f;
        float hbaoBias = 0.35f;
        float hbaoSharpness = 1.0f;

        void verifyParams() const;
    };

    class CameraComponent : public Component {
    public:
        using Component::Component;
        using Params = CameraComponentParams;

        void onAttach(Scene& scene, CameraComponentParams& params);
        void onRenderImGui() override;

        Camera& getCamera();
        bool isPrimary() const;
        void setPrimary(bool primary);

    private:
        Camera camera;
        bool isPrimaryCam;
    };

}
