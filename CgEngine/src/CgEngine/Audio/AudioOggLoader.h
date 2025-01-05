#pragma once

#include "Resources/AudioFile.h"

namespace CgEngine {

    class AudioOggLoader {
    public:
        static AudioFile* loadOggFile(const std::filesystem::path& path);
    };

}
