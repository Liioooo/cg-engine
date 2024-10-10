#pragma once

#include "Mesh.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/PBRMaterial.h"
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
#include "Resources/ResRef.h"

namespace CgEngine {

    class MeshVertices : public Resource, public Mesh {
    public:
        static MeshVertices* createResource(const std::string& name);

        static MeshVertices* createFromPhysx(const glm::vec3* vertices, uint32_t numVertices, const uint32_t* indices, uint32_t numIndices);

        ~MeshVertices() override;

        const std::vector<MeshProps::Vertex>& getVertices() const;
        const std::vector<uint32_t>& getIndexBuffer() const;

        virtual const Material* getMaterial(size_t index) const override;
        virtual const uint32_t getMaterialCount() const override;

        PhysicsTriangleMesh& getPhysicsTriangleMeshForNode(const std::string& nodeName);
        PhysicsConvexMesh& getPhysicsConvexMeshForNode(const std::string& nodeName);

        bool hasSkeleton() const;
        const Skeleton* getSkeleton() const;
        const std::vector<BoneInfo>& getBoneInfos() const;
        const std::unordered_map<std::string, SkeletalAnimation>& getSkeletalAnimations() const;
        const ShaderStorageBuffer* getBoneInfluencesBuffer() const;

        const std::unordered_map<std::string, Animation>& getAnimations() const;


    private:
        MeshVertices() = default;

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

        void importSkeletalAnimations(const aiScene* scene);
        std::optional<SkeletalAnimation> importSkeletalAnimation(const aiAnimation* aiAnimation);

        void importAnimations(const aiScene* scene);
        std::optional<Animation> importAnimation(const aiAnimation* aiAnimation);

        static void importAnimationChannel(AnimationChannel& channel, const aiNodeAnim* nodeAnimation, const aiAnimation* aiAnimation);

        void traverseNodes(aiNode* node, const glm::mat4& parentTransform, int parentNode);
        const MeshNode* findNodeUsingSubmesh(uint32_t submeshIndex) const;

        std::vector<MeshProps::Vertex> vertices;
        std::vector<uint32_t> indexBuffer;
        std::vector<ResRef<PBRMaterial>> materials;
        Skeleton* skeleton = nullptr;
        std::vector<BoneInfluence> boneInfluences{};
        ShaderStorageBuffer* boneInfluencesBuffer = nullptr;
        std::vector<BoneInfo> boneInfos{};
        std::unordered_map<std::string, SkeletalAnimation> skeletalAnimations;
        std::unordered_map<std::string, Animation> animations;
    };

}
