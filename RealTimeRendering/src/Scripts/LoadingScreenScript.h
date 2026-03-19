#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace RTR {

class LoadingScreenScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<LoadingScreenScript>();
        }

        void update(CgEngine::TimeStep ts) override;

    private:
        uint32_t updateCount = 0;
};

}
