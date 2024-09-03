#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"

namespace CgEngine {

    struct CapsuleColliderComponentParams {
        float radius = 1.0f;
        float halfHeight = 0.5f;
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        bool isTrigger = false;
        std::string material = "default-physics-material";

        void verifyParams() const;
    };

    class CapsuleColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, CapsuleColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        PhysicsMaterial& getPhysicsMaterial();
        float getRadius() const;
        float getHalfHeight() const;
        glm::vec3 getOffset() const;
        bool getIsTrigger() const;

    private:
        PhysicsMaterial* physicsMaterial;
        float radius;
        float halfHeight;
        glm::vec3 offset;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
