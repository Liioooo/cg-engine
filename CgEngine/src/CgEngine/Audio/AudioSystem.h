#pragma once

#include <alc.h>
#include <al.h>

namespace CgEngine {

    struct AudioTransform {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 orientation{ 0.0f, 0.0f, -1.0f };
        glm::vec3 up{ 0.0f, 1.0f, 0.0f };

        AudioTransform() = default;
        AudioTransform(glm::quat rotation, glm::vec3 translation) : position(translation), orientation(rotation * glm::vec3(0.0f, 0.0f, -1.0f)), up(rotation * glm::vec3(0.0f, 1.0f, 0.0f)) {}

        bool operator==(const AudioTransform& other) const {
            return position == other.position && orientation == other.orientation && up == other.up;
        }

        bool operator!=(const AudioTransform& other) const {
            return !(*this == other);
        }
    };

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

    private:
        ALCdevice* alcDevice;
        ALCcontext* alcContext;

        AudioListener listener;

        void init();
        void shutdown();
        void update();

        void updateListener();

        static inline AudioSystem* instance;
    };

}
