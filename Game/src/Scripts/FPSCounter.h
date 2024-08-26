#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace Game {

class FPSCounter : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<FPSCounter>();
        }

        void update(CgEngine::TimeStep ts) override;
        void onKeyPressed(CgEngine::KeyPressedEvent& event) override;

    private:
        bool showing = false;
        CgEngine::UiText* fps = nullptr;
        std::array<float, 6> frameTimes;
        uint32_t currentIndex = 0;

        void createUI();
};

}
