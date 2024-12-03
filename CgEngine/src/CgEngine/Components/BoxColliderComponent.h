#pragma once

#include "Component.h"
#include "Physics/PhysicsShape.h"
#include "Resources/ResRef.h"

namespace CgEngine {

    struct BoxColliderComponentParams {
        glm::vec3 halfSize = {0.5f, 0.5f, 0.5f};
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};
        bool isTrigger = false;
        std::string material = "default-physics-material";

        void verifyParams() const;
    };

    class BoxColliderComponent : public Component {
        friend class RigidBodyComponent;
    public:
        using Component::Component;
        using Params = BoxColliderComponentParams;

        void onAttach(Scene& scene, BoxColliderComponentParams& params);
        void onDetach(Scene& scene) override;
        void onRenderImGui() override;

        ResRef<PhysicsMaterial> getPhysicsMaterial();
        glm::vec3 getHalfSize() const;
        glm::vec3 getOffset() const;
        bool getIsTrigger() const;

    private:
        ResRef<PhysicsMaterial> physicsMaterial;
        glm::vec3 halfSize;
        glm::vec3 offset;
        bool isTrigger;
        uint32_t colliderUuid;
    };

}
