#pragma once

#include <Resources/AudioFile.h>
#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Resources/ResRef.h"
#include "Uuid.h"

namespace CgEngine {

    struct AudioComponentParams {
        std::string assetFile;
        float volume = 1.0f;
        float pitch = 1.0f;
        bool looping = false;

        bool playOnAttach = false;
        bool autoDestroy = false;

        void verifyParams() const;
    };

    class AudioComponent : public Component {
    public:
        using Component::Component;
        using Params = AudioComponentParams;

        void onAttach(Scene& scene, AudioComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        Uuid getUuid() const;

        float getVolume() const;
        void setVolume(float volume);

        float getPitch() const;
        void setPitch(float pitch);

        bool isLooping() const;
        void setLooping(bool looping);

        void play();
        void pause();
        void stop();

        bool isPlaying();
        bool isPaused();
        bool isStopped();

        bool getAutoDestroy() const;

    private:
        ResRef<AudioFile> audioFile;
        Uuid uuid;

        float volume;
        float pitch;
        bool looping;

        bool playOnAttach;
        bool autoDestroy;
    };

}
