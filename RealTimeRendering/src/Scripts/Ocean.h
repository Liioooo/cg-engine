#pragma once

#include <OceanCascade.h>
#include <FastFourierTransform.h>
#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Rendering/DescriptorSet.h"
#include "CgEngine/Rendering/UniformBuffer.h"

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

        void onRenderImGui() override;

        void createMesh();

        void reinitialise();

        void updateMaterial();

        CgEngine::CustomMesh* createPlane(glm::vec2 center, glm::vec2 size, int segments);

    private:
        CgEngine::CustomMesh* mesh = nullptr;
        CgEngine::DescriptorSet* mat = nullptr;
        CgEngine::UniformBuffer* matUniformBuffer;

        struct MaterialUniformBufferData {
            glm::vec3 foamColor;
            float roughness;
            glm::vec3 sssColor;
            float roughnessScale;
            glm::vec3 color;
            float maxGloss;
            float foamBias;
            float foamScale;
            float length0;
            float length1;
            float length2;
        };

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
