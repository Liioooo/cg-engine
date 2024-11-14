#include "AudioListenerComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void AudioListenerComponentParams::verifyParams() const {}

    void AudioListenerComponent::onAttach(Scene& scene, AudioListenerComponentParams& params) {
        active = params.active;
        volume = params.volume;
    }

    void AudioListenerComponent::onDetach(Scene& scene) {

    }

    bool AudioListenerComponent::isActive() const {
        return active;
    }

    void AudioListenerComponent::setActive(bool active) {
        this->active = active;
    }

    float AudioListenerComponent::getVolume() const {
        return volume;
    }

    void AudioListenerComponent::setVolume(float volume) {
        this->volume = volume;
    }

    void AudioListenerComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("AudioListenerComponent")) {
        }
    }
}
