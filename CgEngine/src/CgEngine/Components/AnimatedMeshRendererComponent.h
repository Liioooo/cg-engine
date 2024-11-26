#pragma once

#include "Resources/MeshVertices.h"
#include "Rendering/PBRMaterial.h"
#include "TimeStep.h"
#include "Component.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct AnimatedMeshRendererComponentParams {
        std::string assetFile;
        std::string material;
        bool castShadows = true;
        std::vector<std::string> meshNodes;
        std::string animation;
        float animationSpeed = 1.0f;
        bool autoPlayAnimation = true;
        bool loopAnimation = true;

        void verifyParams() const;
    };

    class AnimatedMeshRendererComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, AnimatedMeshRendererComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        ResRef<MeshVertices> getMeshVertices();
        ResRef<PBRMaterial> getMaterial();
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

        bool isActive() const;
        void setActive(bool a);

        void update(TimeStep ts);

    private:
        ResRef<MeshVertices> mesh;
        ResRef<PBRMaterial> material;
        bool castShadows;
        std::vector<uint32_t> meshNodes;

        const SkeletalAnimation* currentAnimation = nullptr;

        float animationSpeed = 1.0f;
        bool isAnimationPlaying = false;
        bool loopAnimation = true;

        std::vector<glm::mat4> boneTransforms;
        VertexArrayObject* skinnedVAO;

        float animationTime = 0.0f;

        void calculateBoneTransforms(const std::vector<glm::mat4>& localBoneTransforms);
        glm::mat4 calcSingleBoneTransform(uint32_t boneIndex, const std::vector<glm::mat4>& localBoneTransforms);

        bool active = true;
    };

}
