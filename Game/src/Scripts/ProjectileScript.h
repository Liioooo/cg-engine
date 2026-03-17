#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace Game {

    class ProjectileScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<ProjectileScript>();
        }

    protected:
        void update(CgEngine::TimeStep ts) override;

        void onCollisionEnter(CgEngine::EntityHandle other) override;

    private:
        float lifetime = 5.0f;

    };

}
