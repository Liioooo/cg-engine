#pragma once

#include "Component.h"
#include "Physics/PhysicsController.h"

namespace CgEngine {

    struct CharacterControllerComponentParams {
        bool hasGravity = true;
        float stepOffset = 0.0f;
        float stepDownOffset = 0.0f;
        float slopeLimit = 0.0f;

        void verifyParams() const;
    };

    class CharacterControllerComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, CharacterControllerComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        void move(glm::vec3 dir);
        void setPosition(glm::vec3 pos);
        void jump(float strength);
        bool isGrounded();

    private:
        PhysicsController* physicsController;

    };

}
