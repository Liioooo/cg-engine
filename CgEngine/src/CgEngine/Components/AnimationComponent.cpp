#include "AnimationComponent.h"
#include "Scene/Scene.h"
#include "Application.h"
#include "imgui.h"

namespace CgEngine {
    void AnimationComponentParams::verifyParams() const {
        CG_ASSERT(!assetFile.empty(), "AnimationComponentParams: 'assetFile' is required.")
    }

    void AnimationComponent::onAttach(Scene& scene, AnimationComponentParams &params) {
        auto& resourceManager = Application::get().getResourceManager();

        animationSource = resourceManager.getResource<MeshVertices>(params.assetFile);

        CG_ASSERT(!animationSource->getAnimations().empty(), "Asset file does not contain any Animations")

        isAnimationPlaying = params.autoPlayAnimation;
        loopAnimation = params.loopAnimation;
        animationSpeed = params.animationSpeed;

        if (params.animation.empty()) {
            currentAnimation = &animationSource->getAnimations().cbegin()->second;
        } else {
            currentAnimation = &animationSource->getAnimations().at(params.animation);
        }
    }

    void AnimationComponent::setAnimation(const std::string& name) {
        currentAnimation = &animationSource->getAnimations().at(name);
        animationTime = 0.0f;
    }

    void AnimationComponent::setAnimationPlaying(bool playing) {
        isAnimationPlaying = playing;
    }

    void AnimationComponent::setAnimationSpeed(float speed) {
        animationSpeed = speed;
    }

    void AnimationComponent::setLoopAnimation(bool loop) {
        loopAnimation = loop;
    }

    void AnimationComponent::reset() {
        animationTime = 0.0f;
    }

    void AnimationComponent::update(TimeStep ts, TransformComponent& transform) {
        if (!isAnimationPlaying || animationSpeed == 0) {
            return;
        }

        animationTime += ts.getSeconds() * animationSpeed / currentAnimation->getDuration();
        if (loopAnimation) {
            animationTime -= glm::floor(animationTime);
        } else {
            animationTime = glm::clamp(animationTime, 0.0f, 1.0f);
        }

        glm::mat4 localPosition(1.0f);
        glm::mat4 localScale(1.0f);
        glm::mat4 localRotation(1.0f);

        if (!currentAnimation->getChannel().translations.empty()) {
            localPosition = glm::translate(localPosition, currentAnimation->getChannel().getTranslationForAnimationTime(animationTime));
        }

        if (!currentAnimation->getChannel().scales.empty()) {
            localScale = glm::scale(localScale, currentAnimation->getChannel().getScaleForAnimationTime(animationTime));
        }

        if (!currentAnimation->getChannel().rotations.empty()) {
            localRotation = glm::toMat4(currentAnimation->getChannel().getRotationForAnimationTime(animationTime));
        }

        transform.setLocalModalMatrix(localPosition * localRotation * localScale);
    }

    void AnimationComponent::onRenderImGui() {
        if (ImGui::CollapsingHeader("AnimationComponent")) {
            if (ImGui::BeginCombo("Animation", currentAnimation->getName().c_str())) {
                for (const auto& item: animationSource->getAnimations()) {
                    if (ImGui::Selectable(item.first.c_str(), currentAnimation == &item.second)) {
                        setAnimation(item.first);
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Is Playing", &isAnimationPlaying);
            ImGui::Checkbox("Loop", &loopAnimation);
            ImGui::SliderFloat("Speed", &animationSpeed, 0.0f, 5.0f);
            ImGui::SliderFloat("Time", &animationTime, 0.0f, 1.0f);
        }
    }
}
