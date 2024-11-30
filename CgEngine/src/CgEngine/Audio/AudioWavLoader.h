#pragma once

#include "Resources/AudioFile.h"

namespace CgEngine {

    class AudioWavLoader {
    public:
        static AudioFile* loadWavFile(const std::filesystem::path& path);
    };

}
