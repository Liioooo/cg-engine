#include "AudioComponent.h"
#include "Scene/Scene.h"
#include "Application.h"

namespace CgEngine {
    void AudioComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty(), "AudioComponentParams: 'assetFile' is required.")
    }

    void AudioComponent::onAttach(Scene& scene, AudioComponentParams& params) {
        auto& resourceManager = Application::get().getResourceManager();
        audioFile = resourceManager.getResource<AudioFile>(params.assetFile);

        volume = params.volume;
        pitch = params.pitch;
        looping = params.looping;

        playOnAttach = params.playOnAttach;
        autoDestroy = params.autoDestroy;

        auto& transform = scene.getComponent<TransformComponent>(entity);
        AudioSystem::get().registerAudioComponent(uuid, audioFile, volume, pitch, looping, {transform.getGlobalRotationQuat(), transform.getGlobalPosition()}, playOnAttach, autoDestroy);
    }

    void AudioComponent::onDetach(Scene& scene) {
        AudioSystem::get().unregisterAudioComponent(uuid);
    }

    Uuid AudioComponent::getUuid() const {
        return uuid;
    }

    float AudioComponent::getVolume() const {
        return volume;
    }

    void AudioComponent::setVolume(float v) {
        volume = v;
    }

    float AudioComponent::getPitch() const {
        return pitch;
    }

    void AudioComponent::setPitch(float p) {
        pitch = p;
    }

    bool AudioComponent::isLooping() const {
        return looping;
    }

    void AudioComponent::setLooping(bool l) {
        looping = l;
    }

    void AudioComponent::play() {
        AudioSystem::get().play(uuid);
    }

    void AudioComponent::pause() {
        AudioSystem::get().pause(uuid);
    }

    void AudioComponent::stop() {
        AudioSystem::get().stop(uuid);
    }

    bool AudioComponent::isPlaying() {
        return AudioSystem::get().isPlaying(uuid);
    }

    bool AudioComponent::isPaused() {
        return AudioSystem::get().isPaused(uuid);
    }

    bool AudioComponent::isStopped() {
        return AudioSystem::get().isStopped(uuid);
    }

    bool AudioComponent::getAutoDestroy() const {
        return autoDestroy;
    }
}
