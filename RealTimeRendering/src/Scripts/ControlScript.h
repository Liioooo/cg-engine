#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace RTR {

    class ControlScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<ControlScript>();
        }

    protected:
        void update(CgEngine::TimeStep ts) override;
        void onKeyPressed(CgEngine::KeyPressedEvent& event) override;

    private:
        bool flyCam = true;
    };

}
