#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace Game {

    class EndScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<EndScript>();
        }

    protected:
        void onAttach() override;

        void onMouseButtonPressed(CgEngine::MouseButtonPressedEvent& event) override;

        void onMouseMoved(CgEngine::MouseMovedEvent& event) override;

    };

}
