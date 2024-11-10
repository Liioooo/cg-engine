#include "AudioSourceManager.h"
#include <al.h>
#include <Asserts.h>

namespace CgEngine {
    void AudioSourceManager::initializeSources(uint32_t maxChannels) {
        sources.reserve(maxChannels);

        for (int i = 0; i < maxChannels; i++) {
            uint32_t source;

            alGenSources(1, &source);
            ALenum error = alGetError();
            if (error == AL_NO_ERROR) {
                sources[source] = true;
            } else {
                break;
            }
        }
    }

    void AudioSourceManager::destroySources() {
        for (const auto& item: sources) {
            alDeleteSources(1, &item.first);
        }
    }

    std::optional<uint32_t> AudioSourceManager::getFreeSource() {
        for (auto& [source, free]: sources) {
            if (free) {
                free = false;
                return source;
            }
        }
        return std::nullopt;
    }

    void AudioSourceManager::setSourceFree(uint32_t source) {
        CG_ASSERT(sources.find(source) != sources.end(), "The source does not exist")
        sources[source] = true;
    }
}
