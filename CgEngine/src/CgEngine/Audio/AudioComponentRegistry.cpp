#include "AudioComponentRegistry.h"

namespace CgEngine {
    void AudioComponentRegistry::add(Uuid uuid, const AudioComponentData& data) {
        std::scoped_lock lock{mutex};
        registry[uuid] = data;
    }

    void AudioComponentRegistry::remove(Uuid uuid) {
        std::scoped_lock lock{mutex};
        registry.erase(uuid);
    }

    AudioComponentData* AudioComponentRegistry::get(Uuid uuid) {
        std::shared_lock lock{mutex};
        if (registry.find(uuid) != registry.end()) {
            return &registry[uuid];
        }
        return nullptr;
    }

    std::vector<Uuid> AudioComponentRegistry::getKeys() const {
        std::shared_lock lock{mutex};

        std::vector<Uuid> keys;
        keys.reserve(registry.size());

        for (const auto& item: registry) {
            keys.emplace_back(item.first);
        }
        return keys;
    }

    void AudioComponentRegistry::setPlayState(Uuid uuid, PlayState playState) {
        std::scoped_lock lock{mutex};
        if (registry.find(uuid) != registry.end()) {
            registry[uuid].playState = playState;
        }
    }

    PlayState AudioComponentRegistry::getPlayState(Uuid uuid) const {
        std::shared_lock lock{mutex};
        if (registry.find(uuid) != registry.end()) {
            return registry.at(uuid).playState;
        }
        return PlayState::Initial;
    }
}
