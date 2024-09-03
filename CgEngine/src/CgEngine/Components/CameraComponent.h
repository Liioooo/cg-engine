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

        void verifyParams() const;
    };

    class CameraComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, CameraComponentParams& params);

        Camera& getCamera();
        bool isPrimary() const;
        void setPrimary(bool primary);

    private:
        Camera camera;
        bool isPrimaryCam;
    };

}
