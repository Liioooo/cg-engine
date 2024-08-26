#pragma once

#include "Resources/MeshVertices.h"
#include "Rendering/Material.h"
#include "TimeStep.h"
#include "Component.h"

namespace CgEngine {

    struct AnimatedMeshRendererComponentParams {
        std::string assetFile;
        std::string material;
        bool castShadows;
        std::vector<std::string> meshNodes;
        std::string animation;
        float animationSpeed;
        bool autoPlayAnimation;
        bool loopAnimation;
    };

    class AnimatedMeshRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, AnimatedMeshRendererComponentParams& params);
        void onDetach(Scene& scene) override;

        MeshVertices& getMeshVertices();
        Material* getMaterial();
        bool getCastShadows() const;
        void setCastShadows(bool value);
        const std::vector<uint32_t>& getMeshNodes();
        const std::vector<glm::mat4>& getBoneTransforms();
        VertexArrayObject* getSkinnedVAO();

        void setAnimation(const std::string& name);
        void setAnimationPlaying(bool playing);
        void setAnimationSpeed(float speed);
        void setLoopAnimation(bool loop);
        void reset();

        void update(TimeStep ts);

    private:
        MeshVertices* mesh;
        Material* material;
        bool castShadows;
        std::vector<uint32_t> meshNodes;

        const Animation* currentAnimation = nullptr;

        float animationSpeed = 1.0f;
        bool isAnimationPlaying = false;
        bool loopAnimation = true;

        std::vector<glm::mat4> boneTransforms;
        VertexArrayObject* skinnedVAO;

        float animationTime = 0.0f;

        template<typename T>
        uint32_t getCurrentKeyFrameIndex(const std::vector<AnimationKeyFrame<T>>& keyFrames);

        template<typename T>
        T interpolate(uint32_t frameIndex, const std::vector<AnimationKeyFrame<T>>& keyFrames, const std::function<T(const T&, const T&, float)>& interpolateFn);

        void calculateBoneTransforms(const std::vector<glm::mat4>& localBoneTransforms);
        glm::mat4 calcSingleBoneTransform(uint32_t boneIndex, const std::vector<glm::mat4>& localBoneTransforms);
    };

}
