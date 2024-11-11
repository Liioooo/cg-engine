#include "Ocean.h"

namespace RTR {

    void Ocean::createMesh() {
        mesh = new CgEngine::CustomMesh();

        std::vector<CgEngine::MeshProps::Vertex> vertices;
        std::vector<uint32_t> indices;

        // TODO: This needs to be done in a tesselation shader anyhow
        int segmentsX = 256 - 1;
        int segmentsY = 256 - 1;
        float wh = 250 / 2.0;
        float dh = 250 / 2.0;

        float dx = 1.0 / segmentsX;
        float dy = 1.0 / segmentsY;

        for (int y = 0; y < segmentsY + 1; ++y) {
            for (int x = 0; x < segmentsX + 1; ++x) {
                vertices.emplace_back(-wh + x * dx * (2 * wh), 0, -dh + y * dy * (2 * dh), 0, 1, 0, 0 + x * dx, 0 + y * dy);
            }
        }

        for (int y = 0; y < segmentsY; ++y) {
            for (int x = 0; x < segmentsX; ++x) {
                indices.push_back((x + 1) + y * (segmentsX + 1));
                indices.push_back(x + y * (segmentsX + 1));
                indices.push_back(x + (y + 1) * (segmentsX + 1));

                indices.push_back((x + 1) + y * (segmentsX + 1));
                indices.push_back(x + (y + 1) * (segmentsX + 1));
                indices.push_back((x + 1) + (y + 1) * (segmentsX + 1));
            }
        }

        mesh->setVertexData(vertices, indices, CgEngine::MeshProps::DEFAULT_VERT_BUFF_LAYOUT);
        mesh->setBoundingBox({-wh, -0.25f, -dh}, {wh, 0.25f, dh});

        if (hasEntityComponent<CgEngine::MeshRendererComponent>()) {
            getComponent<CgEngine::MeshRendererComponent>().setCustomMesh(mesh);
        } else {
            CgEngine::MeshRendererComponentParams meshParams;
            meshParams.customMesh = mesh;
            meshParams.material = "ocean";
            attachComponent<CgEngine::MeshRendererComponent>(meshParams);
        }

        mat = new CgEngine::CustomValMaterial();
        // mat->set("u_Color", glm::vec3(0.0f, 0.6f, 0.1f));
        mat->setTexture2D("tex", *oceanCascade0->timeSpectrum, 0);

        CgEngine::CustomShaderRendererComponentParams params;
        params.shader = "ocean/render";
        params.customMesh = mesh;
        params.instanceCount = 1;
        params.customMaterial = mat;
        params.enableCulling = false;
        params.renderPassOptions.useDirShadowMappingData = true;

        auto& c = attachComponent<CgEngine::CustomShaderRendererComponent>(params);
    }

    void Ocean::onAttach() {
        initialSpectrumShader = CgEngine::CustomComputeShader::createResource("ocean/init-spectrum");
        timeSpectrumShader = CgEngine::CustomComputeShader::createResource("ocean/simulate-ocean");
        conjugateSpectrumShader = CgEngine::CustomComputeShader::createResource("ocean/conjugate");

        oceanCascade0 = new OceanCascade(RTR::OceanParams(), initialSpectrumShader, conjugateSpectrumShader, timeSpectrumShader);

        oceanCascade0->calculateInitialState();
        createMesh();

        onPreRenderCbUuid = addOnPreRenderCallback([](const CgEngine::CameraFrustum& camaraFrustum) {
            CG_LOGGING_DEBUG("On PreRender")
        }, true);
    }

    void Ocean::update(CgEngine::TimeStep ts) {
        currentTime = currentTime + ts.getSeconds();
        oceanCascade0->calculateStateAtTime(currentTime);
    }

    void Ocean::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::Enter) {
            oceanCascade0->calculateInitialState();
        }
    }

    void Ocean::onDetach() {
        removeOnPreRenderCallback(onPreRenderCbUuid);

        delete mesh;
        delete instanceBuffer;
        delete mat;
    }
}
