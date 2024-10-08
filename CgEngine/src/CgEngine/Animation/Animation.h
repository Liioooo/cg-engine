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

        glm::vec3 getTranslationForAnimationTime(float animationTime) const;
        glm::vec3 getScaleForAnimationTime(float animationTime) const;
        glm::quat getRotationForAnimationTime(float animationTime) const;

        template<typename T>
        uint32_t getCurrentKeyFrameIndex(const std::vector<AnimationKeyFrame<T>>& keyFrames, float animationTime) const;

        template<typename T>
        T interpolate(uint32_t frameIndex, float animationTime, const std::vector<AnimationKeyFrame<T>>& keyFrames, const std::function<T(const T&, const T&, float)>& interpolateFn) const;
    };

    class SkeletalAnimation {
    public:
        SkeletalAnimation(std::vector<AnimationChannel> channels, float duration) : channels(std::move(channels)), duration(duration) {};
        SkeletalAnimation(SkeletalAnimation&& other) : channels(std::move(other.channels)), duration(other.duration) {};

        float getDuration() const;
        const std::vector<AnimationChannel>& getChannels() const;

    private:
        std::vector<AnimationChannel> channels;
        float duration;
    };

    class Animation {
    public:
        Animation(AnimationChannel channel, float duration, glm::mat4 animationTransform) : channel(std::move(channel)), duration(duration), animationTransform(animationTransform) {};
        Animation(Animation&& other) : channel(std::move(other.channel)), duration(other.duration), animationTransform(other.animationTransform) {};

        float getDuration() const;
        const AnimationChannel& getChannel() const;
        const glm::mat4& getAnimationTransform() const;

    private:
        AnimationChannel channel;
        float duration;
        glm::mat4 animationTransform;
    };

}
