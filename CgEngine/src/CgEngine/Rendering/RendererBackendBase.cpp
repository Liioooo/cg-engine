#include "RendererBackendBase.h"

namespace CgEngine {
    std::tuple<std::vector<RendererBackendBase::QuadVertex>, std::vector<uint32_t>, std::vector<VertexBufferElement>> RendererBackendBase::getUnitQuadVerticesAndIndices() const {
        std::vector<QuadVertex> vertices;
        vertices.resize(4);
        vertices[0].pos = {-1.0f, -1.0f, 0.0f};
        vertices[0].uv = {0.0f, 0.0f};

        vertices[1].pos = {1.0f, -1.0f, 0.0f};
        vertices[1].uv = {1.0f, 0.0f};

        vertices[2].pos = {1.0f, 1.0f, 0.0f};
        vertices[2].uv = {1.0f, 1.0f};

        vertices[3].pos = {-1.0f, 1.0f, 0.0f};
        vertices[3].uv = {0.0f, 1.0f};

        std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0 };

        std::vector<VertexBufferElement> vertexBufferElements = {{ShaderDataType::Float3, true}, {ShaderDataType::Float2, true}};

        return {vertices, indices, vertexBufferElements};
    }

    std::tuple<std::vector<float>, std::vector<uint32_t>, std::vector<VertexBufferElement>> RendererBackendBase::getUnitCubeVerticesAndIndices() const {
        std::vector<float> vertices = {
            -1.0f, 1.0f, 1.0f, // left_top_front_0
            -1.0f, -1.0f, 1.0f, // left_bottom_front_1
            1.0f, 1.0f, 1.0f, // right_top_front_2
            1.0f, -1.0f, 1.0f, // right_bottom_front_3
            1.0f, 1.0f, -1.0f, // right_top_back_4
            1.0f, -1.0f, -1.0f, // right_bottom_back_5
            -1.0f, 1.0f, -1.0f, // left_top_back_6
            -1.0f, -1.0f, -1.0f, // left_bottom_back_7
        };

        std::vector<uint32_t> indices = {
            7, 5, 4,
            4, 6, 7,
            3, 2, 4,
            4, 5, 3,
            1, 7, 0,
            0, 7, 6,
            0, 2, 3,
            3, 1, 0,
            6, 4, 0,
            0, 4, 2,
            1, 3, 7,
            5, 7, 3
        };

        std::vector<VertexBufferElement> vertexBufferElements = {{ShaderDataType::Float3, false}};

        return {vertices, indices, vertexBufferElements};
    }
}
