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

        float GRASS_PATCH_SIZE = 10;
        uint32_t NUM_GRASS = (32 * 32) * 6;

        float GRASS_LOD_DIST = 25.0f;
        float GRASS_MAX_DIST = 100.0f;

        float GRASS_WIDTH = 0.1f;
        float GRASS_HEIGHT = 1.5f;

        const std::array<glm::vec2, 12> GRASS_POSITIONS = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(0.0f, 10.0f),
                glm::vec2(0.0f, 20.0f),
                glm::vec2(0.0f, 30.0f),
                glm::vec2(10.0f, 0.0f),
                glm::vec2(10.0f, 10.0f),
                glm::vec2(10.0f, 20.0f),
                glm::vec2(10.0f, 30.0f),
                glm::vec2(20.0f, 0.0f),
                glm::vec2(20.0f, 10.0f),
                glm::vec2(20.0f, 20.0f),
                glm::vec2(20.0f, 30.0f),
        };

        // <low, high>
        std::vector<std::pair<CgEngine::Entity, CgEngine::Entity>> grassEntities;

        float currentTime = 0.0f;
        CgEngine::Entity grassContainer;

        CgEngine::CustomValMaterial grassMaterialHigh;
        CgEngine::CustomValMaterial grassMaterialLow;

        CgEngine::ShaderStorageBuffer* offsetsBuffer;
        CgEngine::CustomMesh* geometryHigh;
        CgEngine::CustomMesh* geometryLow;

        void createOffsetsBuffer();
        CgEngine::CustomMesh* createGeometry(uint8_t segments);

    };

}
