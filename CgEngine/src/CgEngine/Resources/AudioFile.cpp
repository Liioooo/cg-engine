#include <Asserts.h>
#include "FileSystem.h"
#include "AudioFile.h"
#include "CgEngineSharedUtils/StringUtils.h"
#include "Audio/AudioWavLoader.h"
#include "Audio/AudioOggLoader.h"
#include <al.h>

namespace CgEngine {
    int AudioFileUtils::getOpenALAudioFormat(CgEngine::AudioFormat format) {
        switch (format) {
            case AudioFormat::Mono8:    return AL_FORMAT_MONO8;
            case AudioFormat::Mono16:   return AL_FORMAT_MONO16;
            case AudioFormat::Stereo8:  return AL_FORMAT_STEREO8;
            case AudioFormat::Stereo16: return AL_FORMAT_STEREO16;
        }
        return 0;
    }

    AudioFile* AudioFile::createResource(const std::string& name) {
        const auto path =  FileSystem::getAsGamePath(name);

        CG_ASSERT(FileSystem::checkFileExists(path), "The AudioFile you are trying to load does not exist")

        std::string extension = FileSystem::getExtension(path);

        if (StringUtils::equalsIgnoreCase(extension, ".wav")) {
            return AudioWavLoader::loadWavFile(path);
        } else if (StringUtils::equalsIgnoreCase(extension, ".ogg")) {
            return AudioOggLoader::loadOggFile(path);
        }

        CG_ASSERT(false, "Unsupported Audio File given")
        return nullptr;
    }

    AudioFile::AudioFile(uint16_t channels, uint64_t sampleRate, uint16_t bitsPerSample) : channels(channels), sampleRate(sampleRate), bitsPerSample(bitsPerSample) {
        if (channels == 1) {
            if (bitsPerSample == 8) {
                format = AudioFormat::Mono8;
            }
            format = AudioFormat::Mono16;
        } else {
            if (bitsPerSample == 8) {
                format = AudioFormat::Stereo8;
            }
            format = AudioFormat::Stereo16;
        }

        CG_ASSERT(bitsPerSample == 8 || bitsPerSample == 16, "AudioFile BitsPerSample must be 8 or 16")
    }

    AudioFile::~AudioFile() {
        if (bitsPerSample == 16) {
            delete[] static_cast<int16_t*>(audioData);
        } else if (bitsPerSample == 8) {
            delete[] static_cast<int8_t*>(audioData);
        }
    }

    void AudioFile::setAudioData(void* data, size_t size) {
        audioData = data;
        dataSize = size;
    }

    AudioFormat AudioFile::getFormat() const {
        return format;
    }

    uint16_t AudioFile::getChannels() const {
        return channels;
    }

    uint64_t AudioFile::getSampleRate() const {
        return sampleRate;
    }

    uint16_t AudioFile::getBitsPerSample() const {
        return bitsPerSample;
    }

    std::pair<const void*, size_t> AudioFile::getAudioData() const {
        return {audioData, dataSize};
    }
}
