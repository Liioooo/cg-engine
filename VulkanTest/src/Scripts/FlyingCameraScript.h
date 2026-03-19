#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace VulkanTest {

    class FlyingCameraScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<FlyingCameraScript>();
        }

    protected:
        void update(CgEngine::TimeStep ts) override;

    private:
        std::pair<float, float> prevMousePos;
        float pitch = 0;
        float yaw = 0;
    };

}
