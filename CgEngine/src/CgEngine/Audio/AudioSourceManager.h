#pragma once

namespace CgEngine {

    class AudioSourceManager {
    public:
        void initializeSources(uint32_t maxChannels);
        void destroySources();

        std::optional<uint32_t> getFreeSource();
        void setSourceFree(uint32_t source);

    private:
        std::unordered_map<uint32_t, bool> sources;

    };

}
