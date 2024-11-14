#pragma once

#include "TimeStep.h"
#include "Resources/MeshVertices.h"
#include "Animation/Animation.h"
#include "Component.h"
#include "Rendering/Camera.h"
#include "TransformComponent.h"

namespace CgEngine {

    struct AnimationComponentParams {
        std::string assetFile;
        std::string animation;

        float animationSpeed = 1.0f;
        bool autoPlayAnimation = true;
        bool loopAnimation = true;

        void verifyParams() const;
    };

    class AnimationComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, AnimationComponentParams& params);
        void onRenderImGui() override;

        void setAnimation(const std::string& name);
        void setAnimationPlaying(bool playing);
        void setAnimationSpeed(float speed);
        void setLoopAnimation(bool loop);
        void reset();

        void update(TimeStep ts, TransformComponent& transform);

    private:
        ResRef<MeshVertices> animationSource;
        const Animation* currentAnimation = nullptr;

        float animationSpeed = 1.0f;
        bool isAnimationPlaying = false;
        bool loopAnimation = true;

        float animationTime = 0.0f;
    };

}
