#pragma once

#include <utility>

#include "Uuid.h"
#include "AudioTransform.h"
#include "Resources/ResRef.h"
#include "Resources/AudioFile.h"

namespace CgEngine {

    enum class PlayState {
        Initial, Playing, Paused, Stopped
    };

    struct AudioComponentData {
        ResRef<AudioFile> audioFile;
        float volume;
        float pitch;
        bool looping;
        AudioTransform transform;
        glm::vec3 velocity = glm::vec3(0.0f);
        bool autoDestroy;

        std::optional<uint32_t> source = std::nullopt;
        PlayState playState = PlayState::Initial;

        AudioComponentData() = default;
        AudioComponentData(ResRef<AudioFile> audioFile, float volume, float pitch, bool looping, const AudioTransform& transform, bool autoDestroy) : audioFile(std::move(audioFile)), volume(volume), pitch(pitch), looping(looping), transform(transform), autoDestroy(autoDestroy) {};
    };

    class AudioComponentRegistry {
    public:
        void add(Uuid uuid, const AudioComponentData& data);
        void remove(Uuid uuid);
        AudioComponentData* get(Uuid uuid);
        std::vector<Uuid> getKeys() const;

        void setPlayState(Uuid uuid, PlayState playState);
        PlayState getPlayState(Uuid uuid) const;

    private:
        mutable std::shared_mutex mutex;
        std::unordered_map<Uuid, AudioComponentData> registry;
    };

}
