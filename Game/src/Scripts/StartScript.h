#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace Game {

    class StartScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<StartScript>();
        }

    protected:
        void onEnable() override;
    };

}
