#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"

namespace CgEngine {

    struct BoxColliderComponentParams {
        glm::vec3 halfSize;
        glm::vec3 offset;
        bool isTrigger;
        std::string material;
    };

    class BoxColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;

        void onAttach(Scene& scene, BoxColliderComponentParams& params);
        void onDetach(Scene& scene) override;

        PhysicsMaterial& getPhysicsMaterial();
        glm::vec3 getHalfSize() const;
        glm::vec3 getOffset() const;
        bool getIsTrigger() const;

    private:
        PhysicsMaterial* physicsMaterial;
        glm::vec3 halfSize;
        glm::vec3 offset;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
