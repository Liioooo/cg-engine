#include "Animation.h"

namespace CgEngine {
    float Animation::getDuration() const {
        return duration;
    }

    const std::vector<AnimationChannel>& Animation::getChannels() const {
        return channels;
    }
}