#include "Ocean.h"

namespace RTR {

    void Ocean::createMesh() {
        mesh = new CgEngine::CustomMesh();

        std::vector<CgEngine::MeshProps::Vertex> vertices;
        std::vector<uint32_t> indices;

        // TODO: This needs to be done in a tesselation shader anyhow
        int segmentsX = 256 - 1;
        int segmentsY = 256 - 1;
        float wh = 400 / 2.0;
        float dh = 400 / 2.0;

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
        mesh->getBoundingBox().addBoxCoordinates({-wh, -0.25f, -dh}, {wh, 0.25f, dh});

        mat = new CgEngine::CustomValMaterial();
        mat->setTexture2D("u_displacementC0", *oceanCascade0->displacement, 10);
        mat->setTexture2D("u_derivativesC0", *oceanCascade0->derivatives, 11);
        mat->setTexture2D("u_turbulenceC0", *oceanCascade0->turbulence, 12);
        mat->setTexture2D("u_displacementC1", *oceanCascade1->displacement, 13);
        mat->setTexture2D("u_derivativesC1", *oceanCascade1->derivatives, 14);
        mat->setTexture2D("u_turbulenceC1", *oceanCascade1->turbulence, 15);
        mat->setTexture2D("u_displacementC2", *oceanCascade2->displacement, 16);
        mat->setTexture2D("u_derivativesC2", *oceanCascade2->derivatives, 17);
        mat->setTexture2D("u_turbulenceC2", *oceanCascade2->turbulence, 18);

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
        float length0 = 250;
        float length1 = 17;
        float length2 = 5;

        float boundary1 = 2 * glm::pi<float>() / length1 * 6;
        float boundary2 = 2 * glm::pi<float>() / length2 * 6;

        RTR::OceanParams oceanParams0 = {
            256,
            length0,
            500,
            9.81,
            {
                30,
                3.3,
                -1, // Will be calculated
                -1, // Will be calculated
                0.0001f,
                boundary1,
                {0, 0} // Currently not used
            }
        };

        auto oceanParams1 = OceanParams(oceanParams0);
        oceanParams1.length = length1;
        oceanParams1.spectrumParams.cutoffLow = boundary1;
        oceanParams1.spectrumParams.cutoffHigh = boundary2;
        auto oceanParams2 = OceanParams(oceanParams0);
        oceanParams2.length = length2;
        oceanParams2.spectrumParams.cutoffLow = boundary2;
        oceanParams2.spectrumParams.cutoffHigh = 9999;

        oceanCascade0 = new OceanCascade(oceanParams0, CgEngine::Application::get().getResourceManager());
        oceanCascade1 = new OceanCascade(oceanParams1, CgEngine::Application::get().getResourceManager());
        oceanCascade2 = new OceanCascade(oceanParams2, CgEngine::Application::get().getResourceManager());

        oceanCascade0->calculateInitialState();
        oceanCascade1->calculateInitialState();
        oceanCascade2->calculateInitialState();
        createMesh();

        onPreRenderCbUuid = addOnPreRenderCallback([](const CgEngine::CameraFrustum& camaraFrustum) {
            CG_LOGGING_DEBUG("On PreRender")
        }, true);
    }

    void Ocean::update(CgEngine::TimeStep ts) {
        currentTime = currentTime + ts.getSeconds();
        oceanCascade0->calculateStateAtTime(currentTime, ts.getSeconds());
        oceanCascade1->calculateStateAtTime(currentTime, ts.getSeconds());
        oceanCascade2->calculateStateAtTime(currentTime, ts.getSeconds());
    }

    void Ocean::onKeyPressed(CgEngine::KeyPressedEvent& event) {
        if (event.getKeyCode() == CgEngine::KeyCode::Enter) {
            oceanCascade0->calculateInitialState();
            oceanCascade1->calculateInitialState();
            oceanCascade2->calculateInitialState();
        }
        if (event.getKeyCode() == CgEngine::KeyCode::F1) {
            mat->setTexture2D("u_displacement", *oceanCascade0->displacement, 0);
            mat->setTexture2D("u_derivatives", *oceanCascade0->derivatives, 1);
            mat->setTexture2D("u_turbulence", *oceanCascade0->turbulence, 2);
        }
        if (event.getKeyCode() == CgEngine::KeyCode::F2) {
            mat->setTexture2D("u_displacement", *oceanCascade1->displacement, 0);
            mat->setTexture2D("u_derivatives", *oceanCascade1->derivatives, 1);
            mat->setTexture2D("u_turbulence", *oceanCascade1->turbulence, 2);
        }
        if (event.getKeyCode() == CgEngine::KeyCode::F3) {
            mat->setTexture2D("u_displacement", *oceanCascade2->displacement, 0);
            mat->setTexture2D("u_derivatives", *oceanCascade2->derivatives, 1);
            mat->setTexture2D("u_turbulence", *oceanCascade2->turbulence, 2);
        }
    }

    void Ocean::onDetach() {
        removeOnPreRenderCallback(onPreRenderCbUuid);

        delete mesh;
        delete instanceBuffer;
        delete mat;
    }
}
