#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Logging.h"

namespace RTR {

    class PlayerScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<PlayerScript>();
        }

    protected:
        void onEnable() override;
        void fixedUpdate(CgEngine::TimeStep ts) override;
        void lateUpdate(CgEngine::TimeStep ts) override;

        void onMouseScrolled(CgEngine::MouseScrolledEvent& event) override;

    private:
        std::pair<float, float> prevMousePos;
        float pitch = -0.6f;
        float yaw = 0.0f;
        float cameraDistance = 4.0f;

        glm::vec3 cameraOffset = glm::vec3(0.0f, 1.5f, 0.0f);
    };

}
