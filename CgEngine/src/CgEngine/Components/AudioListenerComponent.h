#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct AudioListenerComponentParams {
        bool active = true;
        float volume = 1.0f;

        void verifyParams() const;
    };

    class AudioListenerComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, AudioListenerComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        bool isActive() const;
        void setActive(bool active);

        float getVolume() const;
        void setVolume(float volume);

    private:
        bool active = true;
        float volume;
    };

}
