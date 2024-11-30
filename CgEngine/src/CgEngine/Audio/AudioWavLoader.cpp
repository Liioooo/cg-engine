#include <Asserts.h>
#include "AudioWavLoader.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace CgEngine {
    AudioFile* AudioWavLoader::loadWavFile(const std::filesystem::path& path) {
        drwav wav;

        bool opened = drwav_init_file_with_metadata(&wav, path.string().c_str(), 0, nullptr);
        CG_ASSERT(opened, "WAV file could not be opened")

        auto* audioFile = new AudioFile(wav.channels, wav.sampleRate, 16);

        auto* pcmFrames = new int16_t[wav.totalPCMFrameCount * wav.channels];
        drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcmFrames);
        audioFile->setAudioData(pcmFrames, wav.totalPCMFrameCount * wav.channels * sizeof(int16_t));

        drwav_uninit(&wav);

        return audioFile;
    }
}
