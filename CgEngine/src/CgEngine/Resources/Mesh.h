#pragma once

#include <assimp/scene.h>
#include "Physics/PhysicsTriangleMesh.h"
#include "Physics/PhysicsConvexMesh.h"
#include "Rendering/AABoundingBox.h"
#include "Rendering/PBRMaterial.h"
#include "Rendering/VertexArrayObject.h"

namespace CgEngine {

    namespace MeshProps {
        static inline const std::vector<VertexBufferElement> DEFAULT_VERT_BUFF_LAYOUT = {
                VertexBufferElement(ShaderDataType::Float4, true),
                VertexBufferElement(ShaderDataType::Float4, true),
                VertexBufferElement(ShaderDataType::Float4, true),
                VertexBufferElement(ShaderDataType::Float4, true),
                VertexBufferElement(ShaderDataType::Float4, true)
        };

        static inline const std::vector<VertexBufferLayout> DEFAULT_VERT_BUFF_LAYOUTS = {
                VertexBufferLayout(DEFAULT_VERT_BUFF_LAYOUT)
        };

        struct Vertex {
            glm::vec4 position;
            glm::vec4 normal;
            glm::vec4 tangent;
            glm::vec4 bitangent;
            glm::vec4 uv;

            Vertex() {};
            Vertex(float pX, float pY, float pZ, float nX, float nY, float nZ, float tU, float tV) : position{pX, pY, pZ, 1.0f}, normal{nX, nY, nZ, 0.0f}, uv{tU, tV, 0.0f, 0.0f} {};
            Vertex(float pX, float pY, float pZ, float nX, float nY, float nZ) : position{pX, pY, pZ, 1.0f}, normal{nX, nY, nZ, 0.0f} {};
            Vertex(float pX, float pY, float pZ) : position{pX, pY, pZ, 1.0f} {};
        };
    }

    struct Submesh {
        uint32_t baseVertex;
        uint32_t baseIndex;
        uint32_t indexCount;
        uint32_t vertexCount;
        uint32_t materialIndex;
    };

    // LODs:
    // There are nodes called Mesh_LOD1, Mesh_LOD2, etc. in the scene graph.
    // There is one node created on import/create called Mesh.
    // The "Mesh" node holds a vector of lodMeshNodes referencing the Mesh_LOD1, Mesh_LOD2, etc. nodes.
    // Mesh_LOD1, Mesh_LOD2, etc. nodes have isLodNode set to true.
    // The Mesh_LOD1, Mesh_LOD2, etc. all have the same transform.

    struct MeshNode {
        aiNode* aiNode = nullptr;
        PhysicsTriangleMesh* physicsTriangleMesh = nullptr;
        PhysicsConvexMesh* physicsConvexMesh = nullptr;
        AABoundingBox aaBoundingBox;
        int parentNode;
        std::vector<uint32_t> submeshIndices;
        glm::mat4 localTransform{1.0f};
        glm::mat4 transform{1.0f};
        std::map<uint32_t, uint32_t> lodMeshNodesTempMap = {};
        std::vector<uint32_t> lodMeshNodes = {};
        bool isLodNode = false;
    };

    class Mesh {
    public:
        virtual ~Mesh();

        virtual VertexArrayObject* getVAO();
        virtual const std::vector<Submesh>& getSubmeshes() const;
        virtual std::vector<MeshNode>& getMeshNodes();
        virtual uint32_t getMeshNodeIndex(const std::string& nodeName) const;
        virtual const PBRMaterial* getMaterial(size_t index) const = 0;
        virtual const uint32_t getMaterialCount() const = 0;

    protected:
        VertexArrayObject* vao = nullptr;
        std::vector<Submesh> submeshes;
        std::vector<MeshNode> meshNodes{};
        std::unordered_map<std::string, uint32_t> nodeNameToNode{};
    };

}
