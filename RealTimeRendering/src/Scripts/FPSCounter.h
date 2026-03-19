#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace RTR {

class FPSCounter : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<FPSCounter>();
        }

        void onEnable() override;
        void update(CgEngine::TimeStep ts) override;

    private:
        CgEngine::UiText* fps = nullptr;
        std::array<float, 60> frameTimes;
        uint32_t currentIndex = 0;
};

}
