#pragma once

namespace CgEngine {

    template<typename T>
    struct AnimationKeyFrame {
        float timeStamp;
        T value;

        AnimationKeyFrame(float timeStamp, T value) : timeStamp(timeStamp), value(value) {};
    };
    using AnimationTranslationKeyFrame = AnimationKeyFrame<glm::vec3>;
    using AnimationScaleKeyFrame = AnimationKeyFrame<glm::vec3>;
    using AnimationRotationKeyFrame = AnimationKeyFrame<glm::quat>;

    struct AnimationChannel {
        std::vector<AnimationTranslationKeyFrame> translations;
        std::vector<AnimationScaleKeyFrame> scales;
        std::vector<AnimationRotationKeyFrame> rotations;
        uint32_t boneIndex;

        AnimationChannel() = default;
        AnimationChannel(AnimationChannel&& other) : translations(std::move(other.translations)), scales(std::move(other.scales)), rotations(std::move(other.rotations)), boneIndex(other.boneIndex) {};
    };

    class Animation {
    public:
        Animation(std::vector<AnimationChannel> channels, float duration) : channels(std::move(channels)), duration(duration) {};
        Animation(Animation&& other) : duration(other.duration), channels(std::move(other.channels)) {};

        float getDuration() const;
        const std::vector<AnimationChannel>& getChannels() const;

    private:
        std::vector<AnimationChannel> channels;
        float duration;
    };

}
