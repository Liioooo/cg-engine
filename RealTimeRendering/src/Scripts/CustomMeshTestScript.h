#pragma once

#include "CgEngine/Scripting/NativeScript.h"

namespace RTR {

    class CustomMeshTestScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<CustomMeshTestScript>();
        }

    protected:
        void onAttach() override;

        void onDetach() override;

        void update(CgEngine::TimeStep ts) override;

    private:
        CgEngine::CustomMesh* mesh;
        CgEngine::CustomValMaterial* mat;
        CgEngine::ShaderStorageBuffer* instanceBuffer;

        CgEngine::Uuid onPreRenderCbUuid;
    };

}
