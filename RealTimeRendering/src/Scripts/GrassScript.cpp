#include "GrassScript.h"

namespace RTR {

    void GrassScript::onAttach() {
        createOffsetsBuffer();

        geometryHigh = createGeometry(GRASS_SEGMENTS_HIGH);
        geometryLow = createGeometry(GRASS_SEGMENTS_LOW);

        grassMaterialHigh.set("u_GrassParams", glm::vec2{GRASS_SEGMENTS_HIGH, GRASS_VERTICES_HIGH});
        grassMaterialHigh.set("u_GrassSize", glm::vec2{GRASS_WIDTH, GRASS_HEIGHT});
        grassMaterialHigh.set("u_GrassLOD", glm::vec2{GRASS_LOD_DIST, GRASS_MAX_DIST});


        grassMaterialLow.set("u_GrassParams", glm::vec2{GRASS_SEGMENTS_LOW, GRASS_VERTICES_LOW});
        grassMaterialLow.set("u_GrassSize", glm::vec2{GRASS_WIDTH, GRASS_HEIGHT});
        grassMaterialLow.set("u_GrassLOD", glm::vec2{GRASS_LOD_DIST, GRASS_MAX_DIST});

        grassContainer = createEntity();
        CgEngine::TransformComponentParams p;
        attachComponent<CgEngine::TransformComponent>(grassContainer, p);

        for (const auto& pos: GRASS_POSITIONS) {
            CgEngine::Entity grassTileEntity = createEntity(grassContainer);

            CgEngine::TransformComponentParams transformParams;
            transformParams.position = {pos.x, 0.0f, pos.y};
            attachComponent<CgEngine::TransformComponent>(grassTileEntity, transformParams);

            CgEngine::Entity grassLowEntity = createEntity(grassTileEntity);
            attachComponent<CgEngine::TransformComponent>(grassLowEntity);

            CgEngine::CustomShaderRendererComponentParams rendererLowParams;
            rendererLowParams.customMesh = geometryLow;
            rendererLowParams.customMaterial = &grassMaterialLow;
            rendererLowParams.instanceCount = NUM_GRASS;
            rendererLowParams.shader = "grass/render";
            rendererLowParams.enableCulling = true;
            rendererLowParams.renderPassOptions.useEnvironmentMappingData = true;
            auto& rendererLowComp = attachComponent<CgEngine::CustomShaderRendererComponent>(grassLowEntity, rendererLowParams);
            rendererLowComp.setInstanceBuffer1(offsetsBuffer);

            CgEngine::Entity grassHighEntity = createEntity(grassTileEntity);
            attachComponent<CgEngine::TransformComponent>(grassHighEntity);

            CgEngine::CustomShaderRendererComponentParams rendererHighParams;
            rendererHighParams.customMesh = geometryHigh;
            rendererHighParams.customMaterial = &grassMaterialHigh;
            rendererHighParams.instanceCount = NUM_GRASS;
            rendererHighParams.shader = "grass/render";
            rendererHighParams.enableCulling = true;
            rendererHighParams.renderPassOptions.useEnvironmentMappingData = true;
            auto& rendererHighComp = attachComponent<CgEngine::CustomShaderRendererComponent>(grassHighEntity, rendererHighParams);
            rendererHighComp.setInstanceBuffer1(offsetsBuffer);

            grassEntities.emplace_back(grassLowEntity, grassHighEntity);

        }
    }

    void GrassScript::onDetach() {
        delete offsetsBuffer;
        delete geometryLow;
        delete geometryHigh;
    }

    void GrassScript::update(CgEngine::TimeStep ts) {
        auto& primaryCamPos = getComponent<CgEngine::TransformComponent>(getPrimaryCamaraComponent().getEntity()).getGlobalPosition();

        for (const auto& [lowEntity, highEntity]: grassEntities) {
            auto distance = glm::distance(primaryCamPos, getComponent<CgEngine::TransformComponent>(lowEntity).getGlobalPosition());

            auto& lowRenderer = getComponent<CgEngine::CustomShaderRendererComponent>(lowEntity);
            auto& highRenderer = getComponent<CgEngine::CustomShaderRendererComponent>(highEntity);

            if (distance >= GRASS_MAX_DIST) {
                lowRenderer.setActive(false);
                highRenderer.setActive(false);
            } else if (distance >= GRASS_LOD_DIST) {
                lowRenderer.setActive(true);
                highRenderer.setActive(false);
            } else {
                lowRenderer.setActive(false);
                highRenderer.setActive(true);
            }
        }

        currentTime += ts.getSeconds();

        grassMaterialHigh.set("u_Time", currentTime);
        grassMaterialLow.set("u_Time", currentTime);
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

        offsetsBuffer = new CgEngine::ShaderStorageBuffer();
        offsetsBuffer->setData(offsets.data(), offsets.size() * sizeof(glm::vec2));
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

        auto* mesh = new CgEngine::CustomMesh();
        mesh->setVertexData(vertId, indices, {{CgEngine::ShaderDataType::Int, false}});
        mesh->getBoundingBox().setCenterAndExtents(glm::vec3(0.0f), glm::vec3(GRASS_PATCH_SIZE, 200.0f, GRASS_PATCH_SIZE));

        return mesh;
    }
}
