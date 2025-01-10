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

        struct MaterialParams {
            glm::vec3 foamColor;
            glm::vec3 sssColor;
            glm::vec3 color;
            float roughness;
            float roughnessScale;
            float maxGloss;
            float foamBias;
            float foamScale;
        };

    protected:
        void onAttach() override;

        void onDetach() override;

        void update(CgEngine::TimeStep ts) override;

        void onKeyPressed(CgEngine::KeyPressedEvent& event) override;

        void onRenderImGui();

        void createMesh();

        void reinitialise();

        void updateMaterial();

        CgEngine::CustomMesh* createPlane(glm::vec2 center, glm::vec2 size, int segments);

    private:
        CgEngine::CustomMesh* mesh = nullptr;
        CgEngine::CustomValMaterial* mat = nullptr;
        CgEngine::ShaderStorageBuffer* instanceBuffer = nullptr;

        CgEngine::Uuid onPreRenderCbUuid;

        // Custom begin

        float currentTime = 0;

        OceanParams oceanParams0;
        OceanParams oceanParams1;
        OceanParams oceanParams2;

        MaterialParams materialParams;

        OceanCascade* oceanCascade0 = nullptr;
        OceanCascade* oceanCascade1 = nullptr;
        OceanCascade* oceanCascade2 = nullptr;
    };

}
