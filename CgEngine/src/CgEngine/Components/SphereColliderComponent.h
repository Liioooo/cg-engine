#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct SphereColliderComponentParams {
        float radius = 1.0f;
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        bool isTrigger = false;
        std::string material = "default-physics-material";

        void verifyParams() const;
    };

    class SphereColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;
        using Params = SphereColliderComponentParams;

        void onAttach(Scene& scene, SphereColliderComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        ResRef<PhysicsMaterial> getPhysicsMaterial();
        float getRadius() const;
        glm::vec3 getOffset() const;
        bool getIsTrigger() const;

    private:
        ResRef<PhysicsMaterial> physicsMaterial;
        float radius;
        glm::vec3 offset;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
