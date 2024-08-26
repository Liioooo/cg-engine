#pragma once

#include "Component.h"
#include "Physics/PhysicsController.h"

namespace CgEngine {

    struct CharacterControllerComponentParams {
        bool hasGravity;
        float stepOffset;
        float stepDownOffset;
        float slopeLimit;
    };

    class CharacterControllerComponent : public Component {
    public:
        using Component::Component;

        void onAttach(Scene& scene, CharacterControllerComponentParams& params);
        void onDetach(Scene& scene) override;

        void move(glm::vec3 dir);
        void setPosition(glm::vec3 pos);
        void jump(float strength);
        bool isGrounded();

    private:
        PhysicsController* physicsController;

    };

}
