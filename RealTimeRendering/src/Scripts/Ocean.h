#pragma once

#include <OceanCascade.h>
#include <FastFourierTransform.h>
#include "CgEngine/Scripting/NativeScript.h"

namespace RTR {

    class Ocean : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<Ocean>();
        }

    protected:
        void onAttach() override;

        void onDetach() override;

        void update(CgEngine::TimeStep ts) override;

        void onKeyPressed(CgEngine::KeyPressedEvent& event) override;

        void createMesh();

    private:
        CgEngine::CustomMesh* mesh;
        CgEngine::CustomValMaterial* mat;
        CgEngine::ShaderStorageBuffer* instanceBuffer;

        CgEngine::Uuid onPreRenderCbUuid;

        // Custom begin

        float currentTime = 0;

        CgEngine::CustomComputeShader* initialSpectrumShader;
        CgEngine::CustomComputeShader* timeSpectrumShader;
        CgEngine::CustomComputeShader* conjugateSpectrumShader;

        RTR::OceanCascade* oceanCascade0;
    };

}
