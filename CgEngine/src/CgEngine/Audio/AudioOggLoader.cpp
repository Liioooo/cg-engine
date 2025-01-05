#include <Asserts.h>
#include "AudioOggLoader.h"
#include "stb_vorbis.h"

namespace CgEngine {
    AudioFile* AudioOggLoader::loadOggFile(const std::filesystem::path& path) {
        int16_t* data = nullptr;
        int channels;
        int sampleRate;

        int total_samples = stb_vorbis_decode_filename(path.string().c_str(), &channels, &sampleRate, &data);

        CG_ASSERT(total_samples >= 0, "OGG file could not be opened")

        auto* audioFile = new AudioFile(channels, sampleRate, 16);

        auto* pcmFrames = new int16_t[total_samples];
        memcpy(pcmFrames, data, total_samples * sizeof(int16_t));

        free(data);

        audioFile->setAudioData(pcmFrames, total_samples * sizeof(int16_t));

        return audioFile;
    }
}
