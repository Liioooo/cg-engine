#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"

namespace CgEngine {

    struct SphereColliderComponentParams {
        float radius;
        glm::vec3 offset;
        bool isTrigger;
        std::string material;
    };

    class SphereColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, SphereColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        PhysicsMaterial& getPhysicsMaterial();
        float getRadius() const;
        glm::vec3 getOffset() const;
        bool getIsTrigger() const;

    private:
        PhysicsMaterial* physicsMaterial;
        float radius;
        glm::vec3 offset;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
