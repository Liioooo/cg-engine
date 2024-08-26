#pragma once

#include "Component.h"
#include "Rendering/Camera.h"

namespace CgEngine {

    struct CameraComponentParams {
        std::string projection;
        float cnear;
        float cfar;
        float cfov;
        float orthoSize;
        bool isPrimary;
        float exposure;
        float bloomIntensity;
        float bloomThreshold;
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
