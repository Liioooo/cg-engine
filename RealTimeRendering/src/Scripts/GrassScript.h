#pragma once

#include "CgEngine/Scripting/NativeScript.h"
#include "CgEngine/Resources/ResRef.h"
#include "CgEngine/Resources/CustomMesh.h"

namespace RTR {

    struct GrassData {
        glm::vec2 grassParams;
        glm::vec2 grassSize;
        glm::vec3 grassLOD;
        float time;
        glm::vec3 islandCenter;
        float _padding_0;
        glm::vec2 islandSize;
        float _padding_1[2];
        glm::mat4 grassColor;
    };

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
        float GRASS_TERRAIN_NORMAL_RATIO = 0.4f;

        float GRASS_WIDTH = 0.1f;
        float GRASS_HEIGHT = 1.5f;

        glm::vec3 baseColor1 = glm::vec3(64.0f / 255.0f, 71.0f / 255.0f, 10.0f / 255.0f);
        glm::vec3 baseColor2 = glm::vec3(18.0f / 255.0f, 40.0f / 255.0f, 5.0f / 255.0f);
        glm::vec3 tipColor1 = glm::vec3(169.0f / 255.0f, 139.0f / 255.0f, 44.0f / 255.0f);
        glm::vec3 tipColor2 = glm::vec3(220.0f / 255.0f, 120.0f / 255.0f, 50.0f / 255.0f);

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
        std::vector<std::pair<CgEngine::EntityHandle, CgEngine::EntityHandle>> grassEntities;

        float currentTime = 0.0f;
        CgEngine::EntityHandle grassContainer;

        CgEngine::UniformBuffer* grassMatHigh;
        CgEngine::UniformBuffer* grassMatLow;

        GrassData grassMatHighData;
        GrassData grassMatLowData;

        CgEngine::ImmutableShaderStorageBuffer* offsetsBuffer;
        CgEngine::CustomMesh* geometryHigh;
        CgEngine::CustomMesh* geometryLow;

        CgEngine::DescriptorSet* grassDescriptorSetHigh;
        CgEngine::DescriptorSet* grassDescriptorSetLow;

        CgEngine::ResRef<CgEngine::Texture2D> heightGrassMap;

        void createOffsetsBuffer();
        CgEngine::CustomMesh* createGeometry(uint8_t segments);

    };

}
