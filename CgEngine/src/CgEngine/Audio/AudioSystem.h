#pragma once

#include <alc.h>
#include <al.h>
#include "Uuid.h"
#include "Resources/AudioFile.h"
#include "Resources/ResRef.h"
#include "AudioComponentRegistry.h"
#include "AudioTransform.h"
#include "AudioSourceManager.h"
#include "AudioComponentUpdateData.h"

namespace CgEngine {

    struct AudioListener {
        void setNewTransform(const AudioTransform& t) {
            std::unique_lock lock{mutex};
            if (transform != t) {
                transform = t;
                changed = true;
            }
        }

        void setNewVelocity(const glm::vec3& v) {
            std::unique_lock lock{mutex};
            changed = velocity != v;
            velocity = v;
        }

        void setNewVolume(float v) {
            std::unique_lock lock{mutex};
            if (volume != v) {
                volume = v;
                changed = true;
            }
        }

        AudioTransform getTransform() const {
            std::shared_lock lock{mutex};
            return transform;
        }

        glm::vec3 getVelocity() const {
            std::shared_lock lock{mutex};
            return velocity;
        }

        float getVolume() const {
            std::shared_lock lock{mutex};
            return volume;
        }

        bool hasChanged() const {
            bool c = changed.load();
            changed.store(false);
            return c;
        }

    private:
        mutable std::shared_mutex mutex;
        mutable std::atomic<bool> changed;
        AudioTransform transform;
        glm::vec3 velocity;
        float volume;
    };

    class AudioSystem {
    public:
        AudioSystem();
        ~AudioSystem();

        static AudioSystem& get();

        void updateListenerVolume(float volume);
        void updateListenerPosition(const AudioTransform& transform);
        void updateListenerVelocity(const glm::vec3& velocity);

        void registerAudioComponent(Uuid uuid, ResRef<AudioFile> audioFile, float volume, float pitch, bool looping, const AudioTransform& transfrom, bool startPlay, bool autoDestroy);
        void unregisterAudioComponent(Uuid uuid);

        void updateAudioComponents(std::vector<AudioComponentUpdateData> updateData);

        std::unordered_set<Uuid> getComponentsMarkedForDestroy();

        void play(Uuid uuid);
        void pause(Uuid uuid);
        void stop(Uuid uuid);

        bool isPlaying(Uuid uuid);
        bool isPaused(Uuid uuid);
        bool isStopped(Uuid uuid);

    private:
        ALCdevice* alcDevice;
        ALCcontext* alcContext;

        const uint32_t MAX_CHANNELS = 32;

        AudioListener listener;
        AudioComponentRegistry componentRegistry;
        AudioSourceManager sourceManager;

        std::unordered_map<const AudioFile*, std::pair<uint32_t, uint32_t>> audioFileToBufferAndUseCount;

        std::mutex markedForDestroyMutex;
        std::unordered_set<Uuid> markedForDestroy;

        void init();
        void shutdown();
        void update();

        void updateListener();
        void updateSources();

        void playInternal(AudioComponentData* audioComponent);
        void pauseInternal(AudioComponentData* audioComponent);
        void stopInternal(AudioComponentData* audioComponent);

        void registerAudiComponentInternal(Uuid uuid);
        void unregisterAudioComponentInternal(Uuid uuid);

        void updateAudioComponentsInternal(const std::vector<AudioComponentUpdateData>& updateData);

        uint32_t getBufferForAudioFile(const AudioFile* audioFile);
        void reduceAudioFileUseCount(const AudioFile* audioFile);

        void markForDestroy(Uuid uuid);

        static void debugCallback(ALenum source, ALenum type, ALuint id, ALenum severity, ALsizei length, const ALchar *message, void *userParam);

        static inline AudioSystem* instance;
    };

}
