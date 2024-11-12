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
        CgEngine::CustomMesh* mesh = nullptr;
        CgEngine::CustomValMaterial* mat = nullptr;
        CgEngine::ShaderStorageBuffer* instanceBuffer = nullptr;

        CgEngine::Uuid onPreRenderCbUuid;

        // Custom begin

        float currentTime = 0;

        CgEngine::CustomComputeShader* initialSpectrumShader = nullptr;
        CgEngine::CustomComputeShader* timeSpectrumShader = nullptr;
        CgEngine::CustomComputeShader* conjugateSpectrumShader = nullptr;

        OceanCascade* oceanCascade0 = nullptr;
    };

}
