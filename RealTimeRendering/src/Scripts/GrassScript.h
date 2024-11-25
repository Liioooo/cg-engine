#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Resources/ResRef.h"
#include "CgEngine/Rendering/CustomShaders.h"
#include "CgEngine/Resources/CustomMesh.h"


namespace RTR {

    class GrassScript : public CgEngine::NativeScript {
    public:
        static std::shared_ptr<CgEngine::NativeScript> instantiateScript() {
            return std::make_shared<GrassScript>();
        }

        void onAttach() override;
        void onDetach() override;
        void update(CgEngine::TimeStep ts) override;

    private:
        uint32_t GRASS_SEGMENTS_LOW = 1;
        uint32_t GRASS_SEGMENTS_HIGH = 6;
        uint32_t GRASS_VERTICES_LOW = (GRASS_SEGMENTS_LOW + 1) * 2;
        uint32_t GRASS_VERTICES_HIGH = (GRASS_SEGMENTS_HIGH + 1) * 2;

        uint32_t GRASS_PATCH_SIZE = 10;
        uint32_t NUM_GRASS = (32 * 32) * 3;

        CgEngine::CustomValMaterial grassMaterialHigh;
        CgEngine::CustomValMaterial grassMaterialLow;

        std::pair<CgEngine::CustomMesh*, CgEngine::ShaderStorageBuffer*> geometryHigh;
        std::pair<CgEngine::CustomMesh*, CgEngine::ShaderStorageBuffer*> geometryLow;

        std::pair<CgEngine::CustomMesh*, CgEngine::ShaderStorageBuffer*> createGeometry(uint8_t segments);

    };

}
