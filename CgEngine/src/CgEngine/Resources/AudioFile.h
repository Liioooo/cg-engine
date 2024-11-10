#pragma once

namespace CgEngine {

    enum class AudioFormat {
        Mono8,
        Mono16,
        Stereo8,
        Stereo16
    };

    namespace AudioFileUtils {
        int getOpenALAudioFormat(AudioFormat format);
    }

    class AudioFile {
    public:
        static AudioFile* createResource(const std::string& name);

        AudioFile(uint16_t channels, uint64_t sampleRate, uint16_t bitsPerSample);
        ~AudioFile();

        void setAudioData(void* data, size_t size);

        AudioFormat getFormat() const;
        uint16_t getChannels() const;
        uint64_t getSampleRate() const;
        uint16_t getBitsPerSample() const;
        std::pair<const void*, size_t> getAudioData() const;

    private:
        uint16_t channels;
        uint64_t sampleRate;
        uint16_t bitsPerSample;
        void* audioData;
        size_t dataSize;

        AudioFormat format;
    };

}
