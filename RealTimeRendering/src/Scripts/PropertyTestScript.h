#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Resources/ResRef.h"
#include "CgEngine/Rendering/CustomShaders.h"


namespace RTR {

    class PropertyTestScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<PropertyTestScript>();
        }

        void onAttach() override;
        void update(CgEngine::TimeStep ts) override;

    };

}
