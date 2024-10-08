#include "AnimatedMeshRendererComponent.h"
#include "Application.h"

namespace CgEngine {
    void AnimatedMeshRendererComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty(), "AnimatedMeshRendererComponentParams: 'assetFile' is required.")
    }

    void AnimatedMeshRendererComponent::onAttach(Scene& scene, AnimatedMeshRendererComponentParams &params) {
        auto& resourceManager = Application::get().getResourceManager();

        mesh = resourceManager.getResource<MeshVertices>(params.assetFile);

        CG_ASSERT(!mesh->getSkeletalAnimations().empty(), "Mesh does not contain any Animations")

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
            material = resourceManager.getResource<PBRMaterial>(params.material);
        }

        castShadows = params.castShadows;
        isAnimationPlaying = params.autoPlayAnimation;
        loopAnimation = params.loopAnimation;
        animationSpeed = params.animationSpeed;

        if (params.animation.empty()) {
            currentAnimation = &mesh->getSkeletalAnimations().cbegin()->second;
        } else {
            currentAnimation = &mesh->getSkeletalAnimations().at(params.animation);
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

    ResRef<MeshVertices> AnimatedMeshRendererComponent::getMeshVertices() {
        return mesh;
    }

    ResRef<PBRMaterial> AnimatedMeshRendererComponent::getMaterial() {
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
        currentAnimation = &mesh->getSkeletalAnimations().at(name);
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
                localPosition = glm::translate(localPosition, currentAnimation->getChannels().at(i).getTranslationForAnimationTime(animationTime));
            }

            if (!currentAnimation->getChannels().at(i).scales.empty()) {
                localScale = glm::scale(localScale, currentAnimation->getChannels().at(i).getScaleForAnimationTime(animationTime));
            }

            if (!currentAnimation->getChannels().at(i).rotations.empty()) {
                localRotation = glm::toMat4(currentAnimation->getChannels().at(i).getRotationForAnimationTime(animationTime));
            }

            localBoneTransforms.emplace_back(localPosition * localRotation * localScale);
        }

        calculateBoneTransforms(localBoneTransforms);
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
