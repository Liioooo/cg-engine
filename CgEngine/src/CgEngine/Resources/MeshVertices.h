#pragma once

#include "Rendering/VertexArrayObject.h"
#include "Rendering/Material.h"
#include "Rendering/ShaderStorageBuffer.h"
#include "Physics/PhysicsTriangleMesh.h"
#include "Physics/PhysicsConvexMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/BoneInfluence.h"
#include "Animation/BoneInfo.h"
#include "Animation/Animation.h"
#include "Rendering/AABoundingBox.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace CgEngine {

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

    struct Submesh {
        uint32_t baseVertex;
        uint32_t baseIndex;
        uint32_t indexCount;
        uint32_t vertexCount;
        uint32_t materialIndex;
    };

    struct MeshNode {
        aiNode* aiNode;
        PhysicsTriangleMesh* physicsTriangleMesh = nullptr;
        PhysicsConvexMesh* physicsConvexMesh = nullptr;
        AABoundingBox aaBoundingBox;
        int parentNode;
        std::vector<uint32_t> submeshIndices;
        glm::mat4 localTransform{1.0f};
        glm::mat4 transform{1.0f};
    };

    class MeshVertices {
    public:
        static MeshVertices* createResource(const std::string& name);
        static MeshVertices* createFromPhysx(const glm::vec3* vertices, uint32_t numVertices, const uint32_t* indices, uint32_t numIndices);

        MeshVertices() = default;
        ~MeshVertices();

        VertexArrayObject* getVAO();
        const std::vector<Vertex>& getVertices() const;
        const std::vector<uint32_t>& getIndexBuffer() const;
        const std::vector<Submesh>& getSubmeshes() const;
        std::vector<MeshNode>& getMeshNodes();
        uint32_t getMeshNodeIndex(const std::string& nodeName) const;
        const Material* getMaterial(size_t index) const;

        PhysicsTriangleMesh& getPhysicsTriangleMeshForNode(const std::string& nodeName);
        PhysicsConvexMesh& getPhysicsConvexMeshForNode(const std::string& nodeName);

        bool hasSkeleton() const;
        const Skeleton* getSkeleton() const;
        const std::vector<BoneInfo>& getBoneInfos() const;
        const std::unordered_map<std::string, Animation>& getAnimations() const;
        const ShaderStorageBuffer* getBoneInfluencesBuffer() const;

    private:
        static MeshVertices* createCubeMesh();
        static MeshVertices* createSphereMesh(uint32_t latSegments, uint32_t lonSegments);
        static MeshVertices* createCapsuleMesh(float height, float radius);
        static MeshVertices* loadMeshAsset(const std::string& path);

        static std::string getTexturePath(const std::string& modelPath, const std::string& texturePath);
        static glm::mat4 getTransformFromAssimpTransform(const aiMatrix4x4& transform);
        static glm::vec3 getVec3FromAssimpVec(const aiVector3D& vec);
        static glm::quat getQuatFromAssimpQuat(const aiQuaternion& quat);
        static TextureWrap getTextureWrapFromAssimp(aiTextureMapMode mapMode);

        static Skeleton* importSkeleton(const aiScene* scene, MeshVertices* mesh);
        static void traverseNodesBone(const aiNode* node, Skeleton* skeleton, const std::unordered_set<std::string_view>& bones);
        static void traverseBone(const aiNode* node, Skeleton* skeleton, uint32_t parentBone);

        static void importAnimations(const aiScene* scene, const Skeleton* skeleton, std::unordered_map<std::string, Animation>& animations);
        static Animation importAnimation(const aiAnimation* aiAnimation, const Skeleton* skeleton);

        void traverseNodes(aiNode* node, const glm::mat4& parentTransform, int parentNode);
        const MeshNode* findNodeUsingSubmesh(uint32_t submeshIndex) const;

        VertexArrayObject* vao;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indexBuffer;
        std::vector<Submesh> submeshes;
        std::vector<std::unique_ptr<Material>> materials;
        std::vector<MeshNode> meshNodes{};
        std::unordered_map<std::string, uint32_t> nodeNameToNode{};
        Skeleton* skeleton = nullptr;
        std::vector<BoneInfluence> boneInfluences{};
        ShaderStorageBuffer* boneInfluencesBuffer = nullptr;
        std::vector<BoneInfo> boneInfos{};
        std::unordered_map<std::string, Animation> animations;
    };

}
