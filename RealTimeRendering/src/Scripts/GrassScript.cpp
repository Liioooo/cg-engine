#include "GrassScript.h"

namespace RTR {

    void GrassScript::onAttach() {
        geometryLow = createGeometry(GRASS_SEGMENTS_LOW);
        geometryHigh = createGeometry(GRASS_SEGMENTS_HIGH);

        grassMaterialHigh.set("u_GrassParams", glm::vec2{GRASS_SEGMENTS_HIGH, GRASS_VERTICES_HIGH});
        grassMaterialHigh.set("u_GrassSize", glm::vec2{GRASS_WIDTH, GRASS_HEIGHT});
        grassMaterialHigh.set("u_GrassLOD", glm::vec2{GRASS_LOD_DIST, GRASS_MAX_DIST});


        grassMaterialLow.set("u_GrassParams", glm::vec2{GRASS_SEGMENTS_LOW, GRASS_VERTICES_LOW});
        grassMaterialLow.set("u_GrassSize", glm::vec2{GRASS_WIDTH, GRASS_HEIGHT});
        grassMaterialLow.set("u_GrassLOD", glm::vec2{GRASS_LOD_DIST, GRASS_MAX_DIST});

        grassContainer = createEntity();
        CgEngine::TransformComponentParams p;
        attachComponent<CgEngine::TransformComponent>(grassContainer, p);


        auto e = createEntity(grassContainer);

        CgEngine::TransformComponentParams transformParams;
        transformParams.position = {0.0f, 0.0, 0.0f};
        attachComponent<CgEngine::TransformComponent>(e, transformParams);

        CgEngine::CustomShaderRendererComponentParams rendererParams;
        rendererParams.customMesh = geometryHigh.first;
        rendererParams.customMaterial = &grassMaterialHigh;
        rendererParams.instanceCount = NUM_GRASS;
        rendererParams.shader = "grass/render";
        rendererParams.enableCulling = true;
        rendererParams.renderPassOptions.useEnvironmentMappingData = true;
        auto& rendererComp = attachComponent<CgEngine::CustomShaderRendererComponent>(e, rendererParams);
        rendererComp.setInstanceBuffer1(geometryHigh.second);
    }

    void GrassScript::onDetach() {
        delete geometryLow.first;
        delete geometryLow.second;
        delete geometryHigh.first;
        delete geometryHigh.second;
    }

    void GrassScript::update(CgEngine::TimeStep ts) {
        currentTime += ts.getSeconds();

        grassMaterialHigh.set("u_Time", currentTime);
        grassMaterialLow.set("u_Time", currentTime);
    }

    std::pair<CgEngine::CustomMesh*, CgEngine::ShaderStorageBuffer*> GrassScript::createGeometry(uint8_t segments) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(-static_cast<float>(GRASS_PATCH_SIZE) * 0.5f, static_cast<float>(GRASS_PATCH_SIZE) * 0.5f);

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

        std::vector<glm::vec2> offsets;
        offsets.reserve(NUM_GRASS);
        for (uint32_t i = 0; i < NUM_GRASS; ++i) {
            offsets.emplace_back(dist(mt), dist(mt));
        }

        std::vector<int> vertId;
        vertId.resize(vertices * 2);
        for (int i = 0; i < vertices * 2; i++) {
            vertId[i] = i;
        }

        auto* mesh = new CgEngine::CustomMesh();
        mesh->setVertexData(vertId, indices, {{CgEngine::ShaderDataType::Int, false}});
        mesh->getBoundingBox().setCenterAndExtents(glm::vec3(0.0f), glm::vec3(GRASS_PATCH_SIZE, 200.0f, GRASS_PATCH_SIZE));

        auto* positionsBuffer = new CgEngine::ShaderStorageBuffer();
        positionsBuffer->setData(offsets.data(), offsets.size() * sizeof(glm::vec2));

        return {mesh, positionsBuffer};
    }
}
