#include "Animation.h"

namespace CgEngine {
    glm::vec3 AnimationChannel::getTranslationForAnimationTime(float animationTime) const {
        uint32_t index = getCurrentKeyFrameIndex(translations, animationTime);
        return interpolate<glm::vec3>(index, animationTime, translations, [](const glm::vec3& x, const glm::vec3& y, float t) {return glm::mix(x, y, t);});
    }

    glm::vec3 AnimationChannel::getScaleForAnimationTime(float animationTime) const {
        uint32_t index = getCurrentKeyFrameIndex(scales, animationTime);
        return interpolate<glm::vec3>(index, animationTime, scales, [](const glm::vec3& x, const glm::vec3& y, float t) {return glm::mix(x, y, t);});

    }

    glm::quat AnimationChannel::getRotationForAnimationTime(float animationTime) const {
        uint32_t index = getCurrentKeyFrameIndex(rotations, animationTime);
        return rotations.at(index).value;
//        return interpolate<glm::quat>(index, animationTime, rotations, [](const glm::quat& x, const glm::quat& y, float t) {return glm::slerp(x, y, t);});
    }

    template<typename T>
    uint32_t AnimationChannel::getCurrentKeyFrameIndex(const std::vector<AnimationKeyFrame<T>>& keyFrames, float animationTime) const {
        uint32_t index = 0;
        while (keyFrames.at(index).timeStamp < animationTime) {
            index++;
        }
        return index;
    }

    template<typename T>
    T AnimationChannel::interpolate(uint32_t frameIndex, float animationTime, const std::vector<AnimationKeyFrame<T>>& keyFrames, const std::function<T(const T&, const T&, float)>& interpolateFn) const {
        frameIndex = glm::min(frameIndex, static_cast<uint32_t>(keyFrames.size() - 1));
        uint32_t previousIndex = frameIndex == 0 ? 0u : frameIndex - 1;

        float scaleFactor = 0.0f;
        float midWayLength = animationTime - keyFrames.at(previousIndex).timeStamp;
        float framesDiff = keyFrames.at(frameIndex).timeStamp - keyFrames.at(previousIndex).timeStamp;
        scaleFactor = midWayLength / framesDiff;

        return interpolateFn(keyFrames.at(previousIndex).value, keyFrames.at(frameIndex).value, scaleFactor);
    }

    float SkeletalAnimation::getDuration() const {
        return duration;
    }

    const std::vector<AnimationChannel>& SkeletalAnimation::getChannels() const {
        return channels;
    }

    float Animation::getDuration() const {
        return duration;
    }

    const AnimationChannel& Animation::getChannel() const {
        return channel;
    }

    const glm::mat4& Animation::getAnimationTransform() const {
        return animationTransform;
    }


}
