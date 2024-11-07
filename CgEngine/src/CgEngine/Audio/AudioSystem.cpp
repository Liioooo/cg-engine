#include <Logging.h>
#include "AudioSystem.h"
#include "AudioThread.h"

namespace CgEngine {
    AudioSystem::AudioSystem() {
        AudioThread::setOnUpdateCallback<&AudioSystem::update>(this);
        AudioThread::setOnShutdownCallback<&AudioSystem::shutdown>(this);
        AudioThread::start();
        AudioThread::addTask([this] { init(); });
        AudioSystem::instance = this;
    }

    AudioSystem::~AudioSystem() {
        AudioThread::stop();
    }

    AudioSystem& AudioSystem::get() {
        return *AudioSystem::instance;
    }

    void AudioSystem::updateListenerVolume(float volume) {
        listener.setNewVolume(volume);
    }

    void AudioSystem::updateListenerPosition(const AudioTransform& transform) {
        listener.setNewTransform(transform);
    }

    void AudioSystem::updateListenerVelocity(const glm::vec3& velocity) {
        listener.setNewVelocity(velocity);
    }

    void AudioSystem::init() {
        alcDevice = alcOpenDevice(nullptr);

        if (!alcDevice) {
            CG_LOGGING_ERROR("AudioSystem: Device creation failed!")
        }

        alcContext = alcCreateContext(alcDevice, nullptr);
        if (!alcContext) {
            CG_LOGGING_ERROR("AudioSystem: Context creation failed!")
        }

        if (!alcMakeContextCurrent(alcContext)) {
            CG_LOGGING_ERROR("AudioSystem: alcMakeContextCurrent failed!")
        }

        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

        const char* deviceName = nullptr;
        if (alcIsExtensionPresent(alcDevice, "ALC_ENUMERATE_ALL_EXT")) {
            deviceName = alcGetString(alcDevice, ALC_ALL_DEVICES_SPECIFIER);
        }
        if (!deviceName) {
            deviceName = alcGetString(alcDevice, ALC_DEVICE_SPECIFIER);
        }
        CG_LOGGING_INFO("AudioSystem: Device: {0}", std::string_view(deviceName))
    }

    void AudioSystem::shutdown() {
        if (!alcMakeContextCurrent(nullptr)) {
            CG_LOGGING_ERROR("AudioSystem: alcMakeContextCurrent(nullptr) failed!")
        }

        alcDestroyContext(alcContext);

        if (!alcCloseDevice(alcDevice)) {
            CG_LOGGING_ERROR("AudioSystem: Device closing failed!")
        }
    }

    void AudioSystem::update() {
        updateListener();
    }

    void AudioSystem::updateListener() {
        if (listener.hasChanged()) {
            const auto& transform = listener.getTransform();

            float orientation[6];

            orientation[0] = transform.orientation.x;
            orientation[1] = transform.orientation.y;
            orientation[2] = transform.orientation.z;
            orientation[3] = transform.up.x;
            orientation[4] = transform.up.y;
            orientation[5] = transform.up.z;

            alListenerf(AL_GAIN, glm::clamp(listener.getVolume(), 0.0f, 1.0f));
            alListenerfv(AL_POSITION, glm::value_ptr(transform.position));
            alListenerfv(AL_ORIENTATION, orientation);
            alListenerfv(AL_VELOCITY, glm::value_ptr(listener.getVelocity()));
        }
    }
}
