#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Components/UiCanvasComponent2D.h"
#include "CgEngine/Scene/ComponentHandle.h"

namespace Game {

class FPSCounter : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<FPSCounter>();
        }

        void onEnable() override;
        void update(CgEngine::TimeStep ts) override;
        void onKeyPressed(CgEngine::KeyPressedEvent& event) override;

    private:
        CgEngine::ComponentHandle<CgEngine::UiCanvasComponent2D> canvasComponent;
        bool showing = false;
        CgEngine::UiText* fps = nullptr;
        std::array<float, 6> frameTimes;
        uint32_t currentIndex = 0;

        void createUI();
};

}
