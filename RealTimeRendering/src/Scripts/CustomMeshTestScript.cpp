#include "CustomMeshTestScript.h"

namespace RTR {
    void CustomMeshTestScript::update(CgEngine::TimeStep ts) {}

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

        mesh->setVertexData(vertices, indices, CgEngine::MeshProps::DEFAULT_VERT_BUFF_LAYOUT);
        mesh->setBoundingBox({-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});

        getComponent<CgEngine::MeshRendererComponent>().setCustomMesh(mesh);
    }

    void CustomMeshTestScript::onDetach() {
        delete mesh;
    }
}
