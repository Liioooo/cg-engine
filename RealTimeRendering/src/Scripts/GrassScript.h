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

        void onRenderImGui() override;

    private:
        uint32_t GRASS_SEGMENTS_LOW = 1;
        uint32_t GRASS_SEGMENTS_HIGH = 6;
        uint32_t GRASS_VERTICES_LOW = (GRASS_SEGMENTS_LOW + 1) * 2;
        uint32_t GRASS_VERTICES_HIGH = (GRASS_SEGMENTS_HIGH + 1) * 2;

        float GRASS_PATCH_SIZE = 20;
        uint32_t NUM_GRASS = (32 * 32) * 20;

        float GRASS_LOD_DIST = 25.0f;
        float GRASS_MAX_DIST = 200.0f;
        float GRASS_TERRAIN_NORMAL_RATIO = 0.25f;

        float GRASS_WIDTH = 0.1f;
        float GRASS_HEIGHT = 1.5f;

        glm::vec3 baseColor1 = glm::vec3(0.02f, 0.075f, 0.01f);
        glm::vec3 baseColor2 = glm::vec3(0.025f, 0.1f, 0.01f);
        glm::vec3 tipColor1 = glm::vec3(0.65f, 0.8f, 0.25f);
        glm::vec3 tipColor2 = glm::vec3(0.8f, 0.9f, 0.4f);

        const std::array<glm::vec2, 16> GRASS_POSITIONS = {
                glm::vec2(-62.0f, 0.0f),
                glm::vec2(-42.0f, 0.0f),
                glm::vec2(-62.0f, 20.0f),
                glm::vec2(-42.0f, 20.f),
                glm::vec2(-22.0f, 0.0f),
                glm::vec2(-50.0f, -20.0f),
                glm::vec2(-30.0f, -20.0f),
                glm::vec2(-32.0f, -40.0f),
                glm::vec2(-82.0f, 36.0f),
                glm::vec2(-82.0f, 16.0f),
                glm::vec2(-62.0f, 40.0f),
                glm::vec2(-102.0f, 36.0f),
                glm::vec2(-122.0f, 36.0f),
                glm::vec2(-102.0f, 76.0f),
                glm::vec2(-109.0f, 56.0f),
                glm::vec2(-89.0f, 56.0f),
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
