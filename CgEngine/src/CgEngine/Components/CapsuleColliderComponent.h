#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"

namespace CgEngine {

    struct CapsuleColliderComponentParams {
        float radius;
        float halfHeight;
        glm::vec3 offset;
        bool isTrigger;
        std::string material;
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
