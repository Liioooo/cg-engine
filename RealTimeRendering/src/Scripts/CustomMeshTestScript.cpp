#include "CustomMeshTestScript.h"

namespace RTR {
    void CustomMeshTestScript::update(CgEngine::TimeStep ts) {
        CgEngine::Entity e = findEntityById("audio");
        if (hasEntityComponent<CgEngine::AudioComponent>(e)) {
            auto& c = getComponent<CgEngine::AudioComponent>(e);
//            CG_LOGGING_INFO("Playing: {}", c.isPlaying())
//            CG_LOGGING_INFO("Paused: {}", c.isPaused())
        }
    }

    void CustomMeshTestScript::onAttach() {
        mesh = new CgEngine::CustomMesh();

        std::vector<CgEngine::MeshProps::Vertex> vertices;
        std::vector<uint32_t> indices;

        // Front Face
        vertices.emplace_back(-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f); // left_top_front
        vertices.emplace_back(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f); // left_bottom_front
        vertices.emplace_back(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f); // right_top_front
        vertices.emplace_back(0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f); // right_bottom_front

        vertices.emplace_back(0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        // front
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(3);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(4);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(1);

        indices.push_back(0);
        indices.push_back(4);
        indices.push_back(2);

        mesh->setVertexData(vertices, indices, CgEngine::MeshProps::DEFAULT_VERT_BUFF_LAYOUT);
        mesh->setBoundingBox({-0.25f, -0.25f, -0.25f}, {0.25f, 0.25f, 0.25f});

        getComponent<CgEngine::MeshRendererComponent>().setCustomMesh(mesh);

        CgEngine::Entity t = findEntityById("customShaderTest2");

        mat = new CgEngine::CustomValMaterial();
        mat->set("u_Color", glm::vec3(0.0f, 0.6f, 0.1f));

        CgEngine::CustomShaderRendererComponentParams params;
        params.shader = "custom-shader-test2";
        params.assetFile = "CG_CubeMesh";
        params.instanceCount = 10;
        params.customMaterial = mat;
        params.enableCulling = false;
        params.renderPassOptions.useDirShadowMappingData = true;

        std::array<glm::mat4, 10> transforms{};

        for (size_t i = 0; i < transforms.size(); i++) {
            transforms[i] = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f * i, 0.0f)), i * glm::pi<float>() / 4.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        instanceBuffer = new CgEngine::ShaderStorageBuffer();
        instanceBuffer->setData(transforms.data(), transforms.size() * sizeof(glm::mat4));

        auto& c = attachComponent<CgEngine::CustomShaderRendererComponent>(t, params);
        c.setInstanceBuffer1(instanceBuffer);


        auto* tessMesh = new CgEngine::CustomMesh();

        std::vector<glm::vec2> tessVertices;
        std::vector<uint32_t> tessIndices;

        tessVertices.emplace_back(-1.0f, 1.0f);
        tessVertices.emplace_back(-1.0f, -1.0f);
        tessVertices.emplace_back(1.0f, 1.0f);
        tessVertices.emplace_back(1.0f, -1.0f);

        tessIndices.push_back(0);
        tessIndices.push_back(1);
        tessIndices.push_back(2);
        tessIndices.push_back(3);

        tessMesh->setVertexData(tessVertices, tessIndices, {{CgEngine::ShaderDataType::Float2, false}});

        CgEngine::CustomShaderRendererComponentParams tessParams;
        tessParams.shader = "tess-test";
        tessParams.instanceCount = 10;
        tessParams.customMaterial = nullptr;
        tessParams.enableCulling = false;
        tessParams.customMesh = tessMesh;
        tessParams.renderPassOptions.useDirShadowMappingData = false;
        tessParams.renderPassOptions.useEnvironmentMappingData = false;
        tessParams.renderPassOptions.wireframe = true;
        tessParams.renderPassOptions.tesselationPatchSize = 4;

        CgEngine::Entity tessE = findEntityById("tess-test");
        auto& c2 = attachComponent<CgEngine::CustomShaderRendererComponent>(tessE, tessParams);
        c2.setInstanceBuffer1(instanceBuffer);

        onPreRenderCbUuid = addOnPreRenderCallback([](const CgEngine::CameraFrustum& camaraFrustum) {
            CG_LOGGING_DEBUG("On PreRender")
        }, true);
    }

    void CustomMeshTestScript::onDetach() {
        removeOnPreRenderCallback(onPreRenderCbUuid);

        delete mesh;
        delete instanceBuffer;
        delete mat;
    }

    void CustomMeshTestScript::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::M) {
            detachComponent<CgEngine::AudioComponent>();
        }
        if (event.getKeyCode() == CgEngine::KeyCode::N) {
            CgEngine::AudioComponentParams p;
            p.assetFile = "sound.wav";
            p.playOnAttach = true;
            p.looping = true;

            attachComponent<CgEngine::AudioComponent>(p);
        }
        if (event.getKeyCode() == CgEngine::KeyCode::P) {
            CgEngine::Entity e = findEntityById("audio");
            if (hasEntityComponent<CgEngine::AudioComponent>(e)) {
                auto& c = getComponent<CgEngine::AudioComponent>(e);
                if (c.isPaused()) {
                    c.play();
                }
                if (c.isPlaying()) {
                    c.pause();
                }
            }
        }

        if (event.getKeyCode() == CgEngine::KeyCode::KPAdd) {
            CgEngine::Entity e = findEntityById("audio");
            if (hasEntityComponent<CgEngine::AudioComponent>(e)) {
                auto& c = getComponent<CgEngine::AudioComponent>(e);
                c.setVolume(c.getVolume() + 0.1f);
            }
        }
        if (event.getKeyCode() == CgEngine::KeyCode::KPSubtract) {
            CgEngine::Entity e = findEntityById("audio");
            if (hasEntityComponent<CgEngine::AudioComponent>(e)) {
                auto& c = getComponent<CgEngine::AudioComponent>(e);
                c.setVolume(c.getVolume() - 0.1f);
            }
        }
    }
}
