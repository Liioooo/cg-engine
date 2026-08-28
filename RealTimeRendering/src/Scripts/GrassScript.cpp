#include "GrassScript.h"
#include "CgEngine/FileSystem.h"
#include "CgEngine/Rendering/GraphicsObjectsFactory.h"
#include "imgui.h"
#include "Components/CustomShaderRendererComponent.h"
#include "Components/TransformComponent.h"

namespace RTR {

    void GrassScript::onEnable() {
        createOffsetsBuffer();

        geometryHigh = createGeometry(GRASS_SEGMENTS_HIGH);
        geometryLow = createGeometry(GRASS_SEGMENTS_LOW);

        heightGrassMap = getResource<CgEngine::Texture2D>(CgEngine::FileSystem::getAsGamePath("./textures/island_height_grass_map.png").string());

        CgEngine::EntityHandle islandEntity = getParentEntity();
        glm::vec3 islandCenter = islandEntity.getComponent<CgEngine::TransformComponent>()->getGlobalPosition();
        glm::vec2 islandSize = glm::vec2(400.0f, 400.0f);

        glm::mat4 grassColorsUniform = glm::mat4(glm::vec4(baseColor1, 0.0f), glm::vec4(baseColor2, 0.0f), glm::vec4(tipColor1, 0.0f), glm::vec4(tipColor2, 0.0f));

        grassMatHighData.grassParams = glm::vec2{GRASS_SEGMENTS_HIGH, GRASS_VERTICES_HIGH};
        grassMatHighData.grassSize = glm::vec2{GRASS_WIDTH, GRASS_HEIGHT};
        grassMatHighData.grassLOD = glm::vec3{GRASS_LOD_DIST, GRASS_MAX_DIST, GRASS_TERRAIN_NORMAL_RATIO};
        grassMatHighData.islandSize = islandSize;
        grassMatHighData.islandCenter = islandCenter;
        grassMatHighData.grassColor = grassColorsUniform;

        grassMatLowData.grassParams = glm::vec2{GRASS_SEGMENTS_LOW, GRASS_VERTICES_LOW};
        grassMatLowData.grassSize = glm::vec2{GRASS_WIDTH, GRASS_HEIGHT};
        grassMatLowData.grassLOD = glm::vec3{GRASS_LOD_DIST, GRASS_MAX_DIST, GRASS_TERRAIN_NORMAL_RATIO};
        grassMatLowData.islandSize = islandSize;
        grassMatLowData.islandCenter = islandCenter;
        grassMatLowData.grassColor = grassColorsUniform;


        grassMatLow = CgEngine::GraphicsObjectsFactory::createUniformBuffer(sizeof(GrassData));
        grassMatHigh = CgEngine::GraphicsObjectsFactory::createUniformBuffer(sizeof(GrassData));

        grassMatLow->setData(&grassMatLowData, sizeof(GrassData));
        grassMatHigh->setData(&grassMatHighData, sizeof(GrassData));

        const auto* grassDescriptorSetLayout = getResource<CgEngine::CustomGraphicsPipeline>("grass/render")->getDescriptorSetLayout();

        CgEngine::DescriptorSetSpecification grassDescSetSpecLow{};
        grassDescSetSpecLow.layout = grassDescriptorSetLayout;
        grassDescSetSpecLow.texture2DBindings = {
            {10, heightGrassMap.get()}
        };
        grassDescSetSpecLow.uboBindings = {
            {2, grassMatLow}
        };
        grassDescSetSpecLow.immutableSsboBindings = {
            {6, offsetsBuffer}
        };
        grassDescriptorSetLow = CgEngine::GraphicsObjectsFactory::createDescriptorSet(grassDescSetSpecLow);

        CgEngine::DescriptorSetSpecification grassDescSetSpecHigh{};
        grassDescSetSpecHigh.layout = grassDescriptorSetLayout;
        grassDescSetSpecHigh.texture2DBindings = {
            {10, heightGrassMap.get()}
        };
        grassDescSetSpecHigh.uboBindings = {
            {2, grassMatHigh}
        };
        grassDescSetSpecHigh.immutableSsboBindings = {
            {6, offsetsBuffer}
        };
        grassDescriptorSetHigh = CgEngine::GraphicsObjectsFactory::createDescriptorSet(grassDescSetSpecHigh);

        grassContainer = createEntity();
        CgEngine::TransformComponentParams p;
        grassContainer.attachComponent<CgEngine::TransformComponent>(p);

        for (const auto& pos: GRASS_POSITIONS) {
            CgEngine::EntityHandle grassTileEntity = createEntity(grassContainer);

            CgEngine::TransformComponentParams transformParams;
            transformParams.position = {pos.x, 0.0f, pos.y};
            grassTileEntity.attachComponent<CgEngine::TransformComponent>(transformParams);

            CgEngine::EntityHandle grassLowEntity = createEntity(grassTileEntity);
            grassLowEntity.attachComponent<CgEngine::TransformComponent>();

            CgEngine::CustomShaderRendererComponentParams rendererLowParams;
            rendererLowParams.customMesh = geometryLow;
            rendererLowParams.descriptorSet = grassDescriptorSetLow;
            rendererLowParams.instanceCount = NUM_GRASS;
            rendererLowParams.pipeline = "grass/render";
            rendererLowParams.enableCulling = true;
            grassLowEntity.attachComponent<CgEngine::CustomShaderRendererComponent>(rendererLowParams);

            CgEngine::EntityHandle grassHighEntity = createEntity(grassTileEntity);
            grassHighEntity.attachComponent<CgEngine::TransformComponent>();

            CgEngine::CustomShaderRendererComponentParams rendererHighParams;
            rendererHighParams.customMesh = geometryHigh;
            rendererHighParams.descriptorSet = grassDescriptorSetHigh;
            rendererHighParams.instanceCount = NUM_GRASS;
            rendererHighParams.pipeline = "grass/render";
            rendererHighParams.enableCulling = true;
            grassHighEntity.attachComponent<CgEngine::CustomShaderRendererComponent>(rendererHighParams);

            grassEntities.emplace_back(grassLowEntity, grassHighEntity);
        }
    }

    void GrassScript::onDetach() {
        delete grassMatHigh;
        delete grassMatLow;
        delete offsetsBuffer;
        delete geometryLow;
        delete geometryHigh;
        delete grassDescriptorSetHigh;
        delete grassDescriptorSetLow;
    }

    void GrassScript::update(CgEngine::TimeStep ts) {
        auto& primaryCamPos = getPrimaryCameraEntity().getComponent<CgEngine::TransformComponent>()->getGlobalPosition();

        for (const auto& [lowEntity, highEntity]: grassEntities) {
            auto distance = glm::distance(primaryCamPos, lowEntity.getComponent<CgEngine::TransformComponent>()->getGlobalPosition());

            auto lowRenderer = lowEntity.getComponent<CgEngine::CustomShaderRendererComponent>();
            auto highRenderer = highEntity.getComponent<CgEngine::CustomShaderRendererComponent>();

            if (distance >= GRASS_MAX_DIST) {
                lowRenderer->setActive(false);
                highRenderer->setActive(false);
            } else if (distance >= GRASS_LOD_DIST) {
                lowRenderer->setActive(true);
                highRenderer->setActive(false);
            } else {
                lowRenderer->setActive(false);
                highRenderer->setActive(true);
            }
        }

        currentTime += ts.getSeconds();

        grassMatHighData.time = currentTime;
        grassMatLowData.time = currentTime;

        grassMatHigh->setData(&grassMatHighData, sizeof(GrassData));
        grassMatLow->setData(&grassMatLowData, sizeof(GrassData));
    }

    void GrassScript::createOffsetsBuffer() {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(-static_cast<float>(GRASS_PATCH_SIZE) * 0.5f, static_cast<float>(GRASS_PATCH_SIZE) * 0.5f);

        std::vector<glm::vec2> offsets;
        offsets.reserve(NUM_GRASS);
        for (uint32_t i = 0; i < NUM_GRASS; ++i) {
            offsets.emplace_back(dist(mt), dist(mt));
        }

        offsetsBuffer = CgEngine::GraphicsObjectsFactory::createImmutableShaderStorageBuffer(offsets.size() * sizeof(glm::vec2), offsets.data());
    }

    CgEngine::CustomMesh* GrassScript::createGeometry(uint8_t segments) {
        const uint32_t vertices = (segments + 1) * 2;

        std::vector<uint32_t> indices;
        indices.resize(segments * 12);
        for (uint32_t i = 0; i < segments; i++) {
            uint32_t vi = i * 2;
            indices[i*12+0] = vi + 0;
            indices[i*12+1] = vi + 1;
            indices[i*12+2] = vi + 2;

            indices[i*12+3] = vi + 2;
            indices[i*12+4] = vi + 1;
            indices[i*12+5] = vi + 3;

            uint32_t fi = vertices + vi;
            indices[i*12+6] = fi + 2;
            indices[i*12+7] = fi + 1;
            indices[i*12+8] = fi + 0;

            indices[i*12+9]  = fi + 3;
            indices[i*12+10] = fi + 1;
            indices[i*12+11] = fi + 2;
        }

        std::vector<int> vertId;
        vertId.resize(vertices * 2);
        for (int i = 0; i < vertices * 2; i++) {
            vertId[i] = i;
        }

        auto* mesh = new CgEngine::CustomMesh({{CgEngine::ShaderDataType::Int, false}});
        mesh->setVertexData(vertId, indices);
        mesh->getBoundingBox().setCenterAndExtents(glm::vec3(0.0f), glm::vec3(GRASS_PATCH_SIZE / 2.0f, 200.0f, GRASS_PATCH_SIZE / 2.0f));
        mesh->buildMeshData();

        return mesh;
    }

    void GrassScript::onRenderImGui() {
        bool changed = false;

        changed |= ImGui::ColorEdit3("Tip Color 1", glm::value_ptr(tipColor1), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::ColorEdit3("Tip Color 2", glm::value_ptr(tipColor2), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::ColorEdit3("Base Color 1", glm::value_ptr(baseColor1), ImGuiColorEditFlags_DisplayRGB);
        changed |= ImGui::ColorEdit3("Base Color 2", glm::value_ptr(baseColor2), ImGuiColorEditFlags_DisplayRGB);

        changed |= ImGui::InputFloat("Grass Height", &GRASS_HEIGHT);
        changed |= ImGui::InputFloat("Grass Width", &GRASS_WIDTH);
        changed |= ImGui::SliderFloat("Terrain Normal Ratio", &GRASS_TERRAIN_NORMAL_RATIO, 0.0f, 1.0f);

        if (changed) {
            glm::mat4 grassColorsUniform = glm::mat4(glm::vec4(baseColor1, 0.0f), glm::vec4(baseColor2, 0.0f), glm::vec4(tipColor1, 0.0f), glm::vec4(tipColor2, 0.0f));

            grassMatHighData.grassColor = grassColorsUniform;
            grassMatLowData.grassColor = grassColorsUniform;

            grassMatHighData.grassSize = glm::vec2{GRASS_WIDTH, GRASS_HEIGHT};
            grassMatLowData.grassSize = glm::vec2{GRASS_WIDTH, GRASS_HEIGHT};

            grassMatHighData.grassLOD = glm::vec3{GRASS_LOD_DIST, GRASS_MAX_DIST, GRASS_TERRAIN_NORMAL_RATIO};
            grassMatLowData.grassLOD = glm::vec3{GRASS_LOD_DIST, GRASS_MAX_DIST, GRASS_TERRAIN_NORMAL_RATIO};

            grassMatHigh->setData(&grassMatHighData, sizeof(GrassData));
            grassMatLow->setData(&grassMatLowData, sizeof(GrassData));
        }
    }
}
