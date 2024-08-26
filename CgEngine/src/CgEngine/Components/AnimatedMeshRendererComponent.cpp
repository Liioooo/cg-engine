#include "AnimatedMeshRendererComponent.h"
#include "GlobalObjectManager.h"

namespace CgEngine {
    void AnimatedMeshRendererComponent::onAttach(Scene& scene, AnimatedMeshRendererComponentParams &params) {
        mesh = GlobalObjectManager::getInstance().getResourceManager().getResource<MeshVertices>(params.assetFile);

        CG_ASSERT(!mesh->getAnimations().empty(), "Mesh does not contain any Animations")

        if (params.meshNodes.empty()) {
            for (uint32_t i = 0; i < mesh->getMeshNodes().size(); i++) {
                if (!mesh->getMeshNodes().at(i).submeshIndices.empty()) {
                    meshNodes.push_back(i);
                }
            }
        } else {
            for (const auto& nodeName: params.meshNodes) {
                uint32_t i = mesh->getMeshNodeIndex(nodeName);
                auto& mN = mesh->getMeshNodes().at(i);
                if (!mN.submeshIndices.empty()) {
                    meshNodes.push_back(i);
                }
            }
        }

        if (params.material.empty()) {
            material = nullptr;
        } else {
            material = GlobalObjectManager::getInstance().getResourceManager().getResource<Material>(params.material);
        }

        castShadows = params.castShadows;
        isAnimationPlaying = params.autoPlayAnimation;
        loopAnimation = params.loopAnimation;
        animationSpeed = params.animationSpeed;

        if (params.animation.empty()) {
            currentAnimation = &mesh->getAnimations().cbegin()->second;
        } else {
            currentAnimation = &mesh->getAnimations().at(params.animation);
        }

        skinnedVAO = new VertexArrayObject();

        auto vertexBuffer = std::make_shared<VertexBuffer>(mesh->getVertices().size() * sizeof(Vertex), VertexBufferUsage::Dynamic);
        vertexBuffer->setLayout(mesh->getVAO()->getVertexBuffers()[0]->getLayout());
        skinnedVAO->addVertexBuffer(vertexBuffer);
        skinnedVAO->useExistingIndexBuffer(mesh->getVAO()->getIndexBufferRendererId(), mesh->getVAO()->getIndexCount());

        boneTransforms.resize(mesh->getBoneInfos().size());

        if (!params.autoPlayAnimation) {
            calculateBoneTransforms(mesh->getSkeleton()->getBoneTransforms());
        }
    }

    void AnimatedMeshRendererComponent::onDetach(Scene& scene) {
        delete skinnedVAO;
    }

    MeshVertices& AnimatedMeshRendererComponent::getMeshVertices() {
        return *mesh;
    }

    Material* AnimatedMeshRendererComponent::getMaterial() {
        return material;
    }

    bool AnimatedMeshRendererComponent::getCastShadows() const {
        return castShadows;
    }

    void AnimatedMeshRendererComponent::setCastShadows(bool value) {
        castShadows = value;
    }

    const std::vector<uint32_t>& AnimatedMeshRendererComponent::getMeshNodes() {
        return meshNodes;
    }

    const std::vector<glm::mat4>& AnimatedMeshRendererComponent::getBoneTransforms() {
        return boneTransforms;
    }

    VertexArrayObject* AnimatedMeshRendererComponent::getSkinnedVAO() {
        return skinnedVAO;
    }

    void AnimatedMeshRendererComponent::setAnimation(const std::string& name) {
        currentAnimation = &mesh->getAnimations().at(name);
        animationTime = 0.0f;
    }

    void AnimatedMeshRendererComponent::setAnimationPlaying(bool playing) {
        isAnimationPlaying = playing;
    }

    void AnimatedMeshRendererComponent::setAnimationSpeed(float speed) {
        animationSpeed = speed;
    }

    void AnimatedMeshRendererComponent::setLoopAnimation(bool loop) {
        loopAnimation = loop;
    }

    void AnimatedMeshRendererComponent::reset() {
        animationTime = 0.0f;
        calculateBoneTransforms(mesh->getSkeleton()->getBoneTransforms());
    }

    void AnimatedMeshRendererComponent::update(TimeStep ts) {
        if (!isAnimationPlaying || animationSpeed == 0) {
            return;
        }

        animationTime += ts.getSeconds() * animationSpeed / currentAnimation->getDuration();
        if (loopAnimation) {
            animationTime -= glm::floor(animationTime);
        } else {
            animationTime = glm::clamp(animationTime, 0.0f, 1.0f);
        }

        std::vector<glm::mat4> localBoneTransforms;
        localBoneTransforms.reserve(mesh->getSkeleton()->getNumBones());

        for (uint32_t i = 0; i < currentAnimation->getChannels().size(); i++) {
            glm::mat4 localPosition(1.0f);
            glm::mat4 localScale(1.0f);
            glm::mat4 localRotation(1.0f);

            if (!currentAnimation->getChannels().at(i).translations.empty()) {
                uint32_t index = getCurrentKeyFrameIndex(currentAnimation->getChannels().at(i).translations);
                localPosition = glm::translate(localPosition, interpolate<glm::vec3>(index, currentAnimation->getChannels().at(i).translations, [](const glm::vec3& x, const glm::vec3& y, float t) {return glm::mix(x, y, t);}));
            }

            if (!currentAnimation->getChannels().at(i).scales.empty()) {
                uint32_t index = getCurrentKeyFrameIndex(currentAnimation->getChannels().at(i).scales);
                localScale = glm::scale(localScale, interpolate<glm::vec3>(index, currentAnimation->getChannels().at(i).scales, [](const glm::vec3& x, const glm::vec3& y, float t) {return glm::mix(x, y, t);}));
            }

            if (!currentAnimation->getChannels().at(i).rotations.empty()) {
                uint32_t index = getCurrentKeyFrameIndex(currentAnimation->getChannels().at(i).rotations);
                localRotation = glm::toMat4(interpolate<glm::quat>(index, currentAnimation->getChannels().at(i).rotations, [](const glm::quat& x, const glm::quat& y, float t) {return glm::slerp(x, y, t);}));
            }

            localBoneTransforms.emplace_back(localPosition * localRotation * localScale);
        }

        calculateBoneTransforms(localBoneTransforms);
    }

    template<typename T>
    uint32_t AnimatedMeshRendererComponent::getCurrentKeyFrameIndex(const std::vector<AnimationKeyFrame<T>>& keyFrames) {
        uint32_t index = 0;
        while (keyFrames.at(index).timeStamp < animationTime) {
            index++;
        }
        return index;
    }

    template<typename T>
    T AnimatedMeshRendererComponent::interpolate(uint32_t frameIndex, const std::vector<AnimationKeyFrame<T>>& keyFrames, const std::function<T(const T&, const T&, float)>& interpolateFn) {
        frameIndex = glm::min(frameIndex, static_cast<uint32_t>(keyFrames.size() - 1));
        uint32_t previousIndex = glm::max(frameIndex - 1, 0u);

        float scaleFactor = 0.0f;
        float midWayLength = animationTime - keyFrames.at(previousIndex).timeStamp;
        float framesDiff = keyFrames.at(frameIndex).timeStamp - keyFrames.at(previousIndex).timeStamp;
        scaleFactor = midWayLength / framesDiff;

        return interpolateFn(keyFrames.at(previousIndex).value, keyFrames.at(frameIndex).value, scaleFactor);
    }

    void AnimatedMeshRendererComponent::calculateBoneTransforms(const std::vector<glm::mat4>& localBoneTransforms) {
        for (uint32_t i = 0; i < boneTransforms.size(); i++) {
            const BoneInfo& boneInfo = mesh->getBoneInfos().at(i);
            boneTransforms[i] = mesh->getSkeleton()->getArmatureTransform(boneInfo.boneIndex) * calcSingleBoneTransform(boneInfo.boneIndex, localBoneTransforms) * boneInfo.offset;
        }
    }

    glm::mat4 AnimatedMeshRendererComponent::calcSingleBoneTransform(uint32_t boneIndex, const std::vector<glm::mat4>& localBoneTransforms) {
        uint32_t parentBoneIndex = mesh->getSkeleton()->getParentBoneIndex(boneIndex);
        if (parentBoneIndex == Skeleton::NoBone) {
            return localBoneTransforms[boneIndex];
        }
        return calcSingleBoneTransform(parentBoneIndex, localBoneTransforms) * localBoneTransforms[boneIndex];
    }
}
