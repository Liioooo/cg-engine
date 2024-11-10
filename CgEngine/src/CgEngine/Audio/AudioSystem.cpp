#include <Logging.h>

#include <utility>

#include "AudioSystem.h"
#include "AudioThread.h"
#include "open_al_extensions.h"
#include "Asserts.h"

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

    void AudioSystem::registerAudioComponent(Uuid uuid, ResRef<AudioFile> audioFile, float volume, float pitch, bool looping, const AudioTransform& transform, bool startPlay, bool autoDestroy) {
        componentRegistry.add(uuid, {std::move(audioFile), volume, pitch, looping, transform, autoDestroy});
        AudioThread::addTask([this, uuid] { registerAudiComponentInternal(uuid); });

        if (startPlay) {
            AudioThread::addTask([this, uuid] { playInternal(componentRegistry.get(uuid)); });
        }
    }

    void AudioSystem::unregisterAudioComponent(Uuid uuid) {
        AudioThread::addTask([this, uuid] { unregisterAudioComponentInternal(uuid); });
    }

    void AudioSystem::updateAudioComponents(std::vector<AudioComponentUpdateData> updateData) {
        AudioThread::addTask([this, u = std::move(updateData)] { updateAudioComponentsInternal(u); });
    }

    std::unordered_set<Uuid> AudioSystem::getComponentsMarkedForDestroy() {
        std::scoped_lock lock{markedForDestroyMutex};
        std::unordered_set<Uuid> out(markedForDestroy);
        markedForDestroy.clear();
        return out;
    }

    void AudioSystem::play(Uuid uuid) {
        AudioThread::addTask([this, uuid] { playInternal(componentRegistry.get(uuid)); });
    }

    void AudioSystem::pause(Uuid uuid) {
        AudioThread::addTask([this, uuid] { pauseInternal(componentRegistry.get(uuid)); });
    }

    void AudioSystem::stop(Uuid uuid) {
        AudioThread::addTask([this, uuid] { stopInternal(componentRegistry.get(uuid)); });
    }

    bool AudioSystem::isPlaying(Uuid uuid) {
        return componentRegistry.getPlayState(uuid) == PlayState::Playing;
    }

    bool AudioSystem::isPaused(Uuid uuid) {
        return componentRegistry.getPlayState(uuid) == PlayState::Paused;
    }

    bool AudioSystem::isStopped(Uuid uuid) {
        return componentRegistry.getPlayState(uuid) == PlayState::Stopped;
    }

    void AudioSystem::init() {
        alcDevice = alcOpenDevice(nullptr);

        if (!alcDevice) {
            CG_LOGGING_ERROR("AudioSystem: Device creation failed!")
        }

        OpenAlExtensions::loadOpenAlExtensions(alcDevice);

        auto debugFlags = ALCint{ALC_CONTEXT_DEBUG_BIT_EXT};

        #ifndef CG_ENABLE_DEBUG_FEATURES
            debugFlags &= ~ALC_CONTEXT_DEBUG_BIT_EXT;
        #endif

        if (!OpenAlExtensions::has_ALC_EXT_debug) {
            debugFlags &= ~ALC_CONTEXT_DEBUG_BIT_EXT;
        }

        const auto attribs = std::array<ALCint, 3>{{
            ALC_CONTEXT_FLAGS_EXT, debugFlags, 0
        }};

        alcContext = alcCreateContext(alcDevice, attribs.data());
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

        #ifdef CG_ENABLE_DEBUG_FEATURES
            if (OpenAlExtensions::has_ALC_EXT_debug) {
                alEnable(AL_DEBUG_OUTPUT_EXT);
                alDebugMessageControlEXT(AL_DONT_CARE_EXT, AL_DONT_CARE_EXT, AL_DEBUG_SEVERITY_LOW_EXT, 0, nullptr, AL_TRUE);
                alDebugMessageCallbackEXT(&AudioSystem::debugCallback, nullptr);
            }
        #else
            alDisable(AL_DEBUG_OUTPUT_EXT);
        #endif

        sourceManager.initializeSources(MAX_CHANNELS);
    }

    void AudioSystem::shutdown() {
        sourceManager.destroySources();

        if (OpenAlExtensions::has_ALC_EXT_debug) {
            alDebugMessageCallbackEXT(nullptr, nullptr);
        }

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
        updateSources();
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

    void AudioSystem::updateSources() {
        for (const auto& uuid: componentRegistry.getKeys()) {
            auto* audioComponent = componentRegistry.get(uuid);
            if (audioComponent->source.has_value()) {
                int32_t sourceState;
                alGetSourcei(audioComponent->source.value(), AL_SOURCE_STATE, &sourceState);

                PlayState playState = PlayState::Initial;
                switch (sourceState) {
                    case AL_PLAYING:
                        playState = PlayState::Playing;
                        break;
                    case AL_PAUSED:
                        playState = PlayState::Paused;
                        break;
                    case AL_STOPPED:
                        playState = PlayState::Stopped;
                        break;
                }
                componentRegistry.setPlayState(uuid, playState);

                if (audioComponent->autoDestroy && playState == PlayState::Stopped) {
                    // Will be destroyed in Main Scene update
                    markForDestroy(uuid);
                    continue;
                }

                alSourcei(audioComponent->source.value(), AL_LOOPING, audioComponent->looping);
                alSourcef(audioComponent->source.value(), AL_GAIN, glm::clamp(audioComponent->volume, 0.0f, 1.0f));
                alSourcef(audioComponent->source.value(), AL_PITCH, audioComponent->pitch);
                alSourcefv(audioComponent->source.value(), AL_POSITION, glm::value_ptr(audioComponent->transform.position));
                alSourcefv(audioComponent->source.value(), AL_DIRECTION, glm::value_ptr(audioComponent->transform.orientation));
                alSourcefv(audioComponent->source.value(), AL_VELOCITY, glm::value_ptr(audioComponent->velocity));
            }
        }
    }

    void AudioSystem::playInternal(AudioComponentData* audioComponent) {
        if (audioComponent->source.has_value()) {
            alSourcePlay(audioComponent->source.value());
        }
    }

    void AudioSystem::pauseInternal(AudioComponentData* audioComponent) {
        if (audioComponent->source.has_value()) {
            alSourcePause(audioComponent->source.value());
        }
    }

    void AudioSystem::stopInternal(AudioComponentData* audioComponent) {
        if (audioComponent->source.has_value()) {
            alSourceStop(audioComponent->source.value());
        }
    }

    void AudioSystem::registerAudiComponentInternal(Uuid uuid) {
        auto* audioComponent = componentRegistry.get(uuid);

        std::optional<uint32_t> source = sourceManager.getFreeSource();

        CG_ASSERT(source.has_value(), "No more sources left") // Need to handle this at some point

        if (!source.has_value()) {
            return;
        }

        audioComponent->source = source;
        uint32_t buffer = getBufferForAudioFile(audioComponent->audioFile.get());

        alSourcei(source.value(), AL_BUFFER, static_cast<int32_t>(buffer));

        alSourcei(audioComponent->source.value(), AL_LOOPING, audioComponent->looping);
        alSourcef(audioComponent->source.value(), AL_GAIN, audioComponent->volume);
        alSourcef(audioComponent->source.value(), AL_PITCH, audioComponent->pitch);
        alSourcefv(audioComponent->source.value(), AL_POSITION, glm::value_ptr(audioComponent->transform.position));
        alSourcefv(audioComponent->source.value(), AL_DIRECTION, glm::value_ptr(audioComponent->transform.orientation));
    }

    void AudioSystem::unregisterAudioComponentInternal(Uuid uuid) {
        auto* audioComponent = componentRegistry.get(uuid);

        if (audioComponent->source.has_value()) {
            alSourceRewind(audioComponent->source.value());
            alSourcei(audioComponent->source.value(), AL_BUFFER, 0);
            sourceManager.setSourceFree(audioComponent->source.value());
        }
        reduceAudioFileUseCount(audioComponent->audioFile.get());
        componentRegistry.remove(uuid);
    }

    void AudioSystem::updateAudioComponentsInternal(const std::vector<AudioComponentUpdateData>& updateData) {
        for (const auto& data: updateData) {
            auto* audioComponent = componentRegistry.get(data.uuid);
            audioComponent->volume = data.volume;
            audioComponent->pitch = data.pitch;
            audioComponent->looping = data.looping;
            audioComponent->transform = data.transform;
            audioComponent->velocity = data.velocity;
        }
    }

    uint32_t AudioSystem::getBufferForAudioFile(const AudioFile* audioFile) {
        if (audioFileToBufferAndUseCount.find(audioFile) != audioFileToBufferAndUseCount.end()) {
            auto& [buffer, useCount] = audioFileToBufferAndUseCount[audioFile];
            useCount++;
            return buffer;
        }
        auto& [buffer, useCount] = audioFileToBufferAndUseCount[audioFile];
        useCount = 1;

        alGenBuffers(1, &buffer);
        alBufferData(buffer, AudioFileUtils::getOpenALAudioFormat(audioFile->getFormat()), audioFile->getAudioData().first, static_cast<int32_t>(audioFile->getAudioData().second), static_cast<int32_t>(audioFile->getSampleRate()));

        return buffer;
    }

    void AudioSystem::reduceAudioFileUseCount(const AudioFile* audioFile) {
       if (audioFileToBufferAndUseCount.find(audioFile) == audioFileToBufferAndUseCount.end()) {
           return;
       }

        auto& [buffer, useCount] = audioFileToBufferAndUseCount[audioFile];
        useCount--;

        if (useCount == 0) {
            alDeleteBuffers(1, &buffer);
            audioFileToBufferAndUseCount.erase(audioFile);
        }
    }

    void AudioSystem::markForDestroy(Uuid uuid) {
        std::scoped_lock lock{markedForDestroyMutex};
        markedForDestroy.insert(uuid);
    }

    void AudioSystem::debugCallback(ALenum source, ALenum type, ALuint id, ALenum severity, ALsizei length, const ALchar *message, void *userParam) {
        const auto msg = std::string_view{message, static_cast<ALuint>(length)};

        std::stringstream stringStream;
        std::string sourceString = "<invalid source>";
        std::string typeString = "<invalid type>";
        std::string severityString = "<invalid severity>";

        switch (source) {
            case AL_DEBUG_SOURCE_API_EXT: sourceString =  "API"; break;
            case AL_DEBUG_SOURCE_AUDIO_SYSTEM_EXT: sourceString =  "Audio System"; break;
            case AL_DEBUG_SOURCE_THIRD_PARTY_EXT: sourceString =  "Third Party"; break;
            case AL_DEBUG_SOURCE_APPLICATION_EXT: sourceString =  "Application"; break;
            case AL_DEBUG_SOURCE_OTHER_EXT: sourceString =  "Other"; break;
        }

        switch (type) {
            case AL_DEBUG_TYPE_ERROR_EXT: typeString = "Error"; break;
            case AL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_EXT: typeString = "Deprecated Behavior"; break;
            case AL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_EXT: typeString = "Undefined Behavior"; break;
            case AL_DEBUG_TYPE_PORTABILITY_EXT: typeString = "Portability"; break;
            case AL_DEBUG_TYPE_PERFORMANCE_EXT: typeString = "Performance"; break;
            case AL_DEBUG_TYPE_MARKER_EXT: typeString = "Marker"; break;
            case AL_DEBUG_TYPE_PUSH_GROUP_EXT: typeString = "Push Group"; break;
            case AL_DEBUG_TYPE_POP_GROUP_EXT: typeString = "Pop Group"; break;
            case AL_DEBUG_TYPE_OTHER_EXT: typeString = "Other"; break;
        }

        switch (severity) {
            case AL_DEBUG_SEVERITY_HIGH_EXT: severityString = "High"; break;
            case AL_DEBUG_SEVERITY_MEDIUM_EXT: severityString = "Medium"; break;
            case AL_DEBUG_SEVERITY_LOW_EXT: severityString = "Low"; break;
            case AL_DEBUG_SEVERITY_NOTIFICATION_EXT: severityString = "Notification"; break;
        }

        stringStream << "OpenAL Error: " << msg;
        stringStream << " [Source = " << sourceString;
        stringStream << ", Type = " << typeString;
        stringStream << ", Severity = " << severityString;
        stringStream << ", ID = " << id << "]";

        CG_LOGGING_WARNING(stringStream.str());
    }
}
