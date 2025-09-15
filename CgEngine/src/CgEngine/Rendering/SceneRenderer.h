#pragma once

#include "RenderPass.h"
#include "Camera.h"
#include "UniformBuffer.h"
#include "VertexArrayObject.h"
#include "ShaderStorageBuffer.h"
#include "Scene/Scene.h"
#include "Renderer.h"
#include "CameraFrustum.h"

namespace CgEngine {

    class Scene;

    struct RenderingStats {
        float skinMeshesTimer = 0.0f;
        float shadowMapTimer = 0.0f;
        float gBufferTimer = 0.0f;
        float hbaoDeinterleavingTimer = 0.0f;
        float hbaoComputeTimer = 0.0f;
        float hbaoReinterleavingTimer = 0.0f;
        float hbaoBlurTimer = 0.0f;
        float pbrTimer = 0.0f;
        float customShaderForwardTimer = 0.0f;
        float customShaderDeferredTimer = 0.0f;
        float skyboxTimer = 0.0f;
        float bloomTimer = 0.0f;
        float screenTimer = 0.0f;
        float uiTimer = 0.0f;
    };

    class SceneRenderer {
    public:
        explicit SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight);
        ~SceneRenderer();

        void setActiveScene(Scene* scene);
        void setViewportSize(uint32_t width, uint32_t height);
        void beginScene(const Camera& camera, glm::mat4 cameraTransform, const SceneLightEnvironment& lightEnvironment, const SceneEnvironment& sceneEnvironment);
        void endScene();
        void submitMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* overrideMaterial, bool castShadows, bool enableCulling, const glm::mat4& transform, const std::vector<float>& lodDistances);
        void submitAnimatedMesh(MeshVertices* mesh, const std::vector<uint32_t>& meshNodes, Material* overrideMaterial, bool castShadows, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, VertexArrayObject* skinnedVAO);
//        void submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* material, bool enableCulling, const AABoundingBox* boundingBox, const glm::mat4& transform, CustomShader* shader, uint32_t instanceCount, CustomShaderRendererComponentRenderPassOptions& renderPassOptions, std::pair<ShaderStorageBuffer*, ShaderStorageBuffer*> instanceBuffers, const std::vector<float>& lodDistances);
        void submitUiElements(const std::unordered_map<std::string, UiElement*>& uiElements);
        void submitPhysicsColliderMesh(MeshVertices* mesh, const glm::mat4& transform);
        void submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, Mesh* mesh, const std::vector<uint32_t>& meshNodes, const glm::mat4& transform);
        void submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, const AABoundingBox& boundingBox, const glm::mat4& transform);
        void submitDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        const CameraFrustum& getCamaraFrustum() const;

        const RenderingStats& getRenderingStats();


    private:
        static const uint32_t MAX_OBJECTS = 20000;

        static const uint32_t maxBones = 200;
        static const uint32_t maxAnimatedComponents = 512;

        Scene* activeScene;
        uint32_t viewportWidth;
        uint32_t viewportHeight;
        float invViewportWidth;
        float invViewportHeight;
        bool needsResize = true;
        bool activeRendering = false;

        struct TransformsOffsetPushConstants {
            int transformsOffset;
        };

        PushConstants* transformOffsetPushConstant;

        RenderPass* dirShadowMapRenderPass;
        Attachment* dirShadowMaps;
        Framebuffer* dirShadowMapFramebuffer;
        ShaderStorageBuffer* dirShadowMapTransformsBuffer;
        DescriptorSet* dirShadowMapDescriptorSet;


        RenderPass* gBufferRenderPass;
        Attachment* gBufferAlbedoRoughnessAttachment;
        Attachment* gBufferEmissionMetallicAttachment;
        Attachment* gBufferWorldNormalsAttachment;
        Attachment* gBufferViewNormalsAttachment;
        Attachment* gBufferDepthAttachment;
        Framebuffer* gBufferFramebuffer;
        ShaderStorageBuffer* gBufferTransformsBuffer;
        DescriptorSet* gBufferDescriptorSet;

        RenderPass* pbrRenderPass;
        RenderPass* screenRenderPass;
        RenderPass* skyboxRenderPass;
        RenderPass* bloomDownSamplePass;
        RenderPass* bloomUpSamplePass;
        RenderPass* uiCirclePass;
        RenderPass* uiRectPass;
        RenderPass* uiTextPass;
        RenderPass* physicsCollidersRenderPass;
        RenderPass* boundingBoxRenderPass;
        RenderPass* normalsDebugRenderPass;
        RenderPass* debugLinesRenderPass;
        RenderPass* customShaderForwardRenderPass;
        RenderPass* customShaderDeferredRenderPass;

        RenderPass* hbaoDeinterleavingRenderPass;
        RenderPass* hbaoReinterleavingRenderPass;
        RenderPass* hbaoBlurRenderPass;


//        ComputeShader hbaoShader;

        Material pbrPassMaterial;
        Material screenMaterial;
        Material skyboxMaterial;
        Material physicsCollidersMaterial;
        Material boundingBoxMaterial;
        Material normalsDebugMaterial;
        Material emptyMaterial;

//        ComputeShader skinningShader;

        std::array<Texture2D*, 7> bloomTextures;

//        Texture2DArray hbaoDeinterleavingDepthTexture;
//        std::array<Texture2DView, 16> hbaoDeinterleavingDepthTextureViews;
        std::array<Framebuffer*, 2> hbaoDeinterleavingFramebuffers;
        glm::uvec3 hbaoWorkGroupSize;
//        Texture2DArray hbaoResultTexture;

        CameraFrustum cameraFrustum;

        void skinMeshes();
        void shadowMapPass();
        void gBufferPass();
        void hbaoDeinterleavingPass();
        void hbaoComputePass();
        void hbaoReinterleavingPass();
        void hbaoBlurPass();
        void pbrPass();
        void customShaderDeferredPass();
        void customShaderForwardPass();
        void skyboxPass();
        void physicsCollidersPass();
        void boundingBoxPass();
        void normalsDebugPass();
        void debugLinesPass();
        void bloomPass();
        void screenPass();
        void uiPass();

        void setupShadowMapData(glm::vec3 dirLightDirection, const glm::mat4& cameraViewProjection, const Camera& camera);
        void setupHBAOData(const glm::mat4& cameraProjection, const Camera& camera);

        struct UBCameraData {
            glm::mat4 viewProjection;
            glm::mat4 invViewProjection;
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 uiProjectionMatrix;
            glm::vec4 position;
            glm::vec4 clipInfo;
            float exposure;
            float bloomIntensity;
            float bloomThreshold;
            float _padding0_;
        };
        UniformBuffer* ubCameraData;

        glm::vec3 cameraPosition;

        struct UBPointLight {
            glm::vec4 position;
            glm::vec4 color;
            float intensity;
            float radius;
            float falloff;
            float _padding0_;
        };

        struct UBSpotLight {
            glm::vec4 position;
            glm::vec4 color;
            glm::vec4 direction;
            float intensity;
            float radius;
            float falloff;
            float innerAngle;
            float outerAngle;
            glm::vec3 _padding0_;
        };

        struct UBLightData {
            glm::vec4 dirLightDirection;
            glm::vec4 dirLightColor;
            float dirLightIntensity;
            int pointLightCount;
            int spotLightCount;
            float _padding0_;
            UBPointLight pointLights[100];
            UBSpotLight spotLights[100];
        };
        UniformBuffer* ubLightData;

        struct UBDirShadowData {
            glm::mat4 lightSpaceMat[4];
            glm::vec4 cascadeSplits;
        };
        UniformBuffer* ubDirShadowData;

        struct UBScreenData {
            glm::vec2 invFullResolution;
            glm::vec2 fullResolution;
            glm::vec2 invHalfResolution;
            glm::vec2 halfResolution;
        };
        UniformBuffer* ubScreenData;

        struct UBHBAOData {
            glm::vec4 perspectiveInfo;
            glm::vec2 invQuarterResolution;
            float radiusToScreen;
            float negInvR2;

            float nDotVBias;
            float aoMultiplier;
            float powExponent;
            bool isOrtho;

            glm::vec4 float2Offsets[16];
            glm::vec4 jitters[16];
        } hbaoData;
        UniformBuffer* ubHBAOData;

        float hbaoSharpness = 1.0f;

        struct MeshKey {
            const VertexArrayObject* voa;
            const uint32_t submeshIndex;
            const uint32_t materialUuid;

            bool operator<(const MeshKey& other) const {
                if (voa < other.voa) return true;
                if (voa > other.voa) return false;
                if (submeshIndex < other.submeshIndex) return true;
                if (submeshIndex > other.submeshIndex) return false;
                return materialUuid < other.materialUuid;
            }
        };

        struct DrawCommand {
            VertexArrayObject* vao;
            const Material* material;
            uint32_t baseIndex;
            uint32_t baseVertex;
            uint32_t indexCount;
            uint32_t instanceCount;
            int transformsBufferOffset;
        };

        std::map<MeshKey, DrawCommand> drawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> meshTransforms;

        std::map<MeshKey, DrawCommand> shadowMapDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> shadowMapMeshTransforms;

        std::map<MeshKey, DrawCommand> physicsCollidersDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> physicsCollidersMeshTransforms;

        std::map<MeshKey, DrawCommand> boundingBoxDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> boundingBoxMeshTransforms;

        std::vector<LineDrawInfo> debugLinesDrawInfoQueue;

        struct SkinningInfo {
            const VertexBuffer* originalVertexBuffer;
            const VertexBuffer* skinnedVertexBuffer;
            const ShaderStorageBuffer* boneInfluencesBuffer;
            uint32_t numVertices;
        };

        std::vector<SkinningInfo> skinningQueue;

        struct CurrentSceneEnvironment {
            uint32_t prefilterMapId;
            uint32_t irradianceMapId;
            float environmentIntensity;
            bool dirLightCastShadows;
        } currentSceneEnvironment;

        struct UiDrawInfo {
            uint32_t circleIndexCount = 0;
            std::vector<UiCircleVertex> circleVertices;
            uint32_t rectIndexCount = 0;
            std::vector<UiRectVertex> rectVertices;
            std::array<const Texture2D*, Renderer::maxTextureSlots> textureSlots{};
            uint32_t filledTextureSlots = 0;

            std::vector<UiTextVertex> textVertices;
            uint32_t textIndexCount = 0;
            std::array<const Texture2D*, 4> fontAtlases;
            uint32_t filledFontAtlases = 0;
        };

        std::map<uint32_t, UiDrawInfo> uiDrawInfoQueue;

        glm::mat4 uiProjectionMatrix;

//        ShaderStorageBuffer boneTransformsBuffer{false};

//        struct CustomShaderDrawCommand {
//            uint32_t instanceCount;
//            VertexArrayObject* vao;
//            const Material* material;
//            uint32_t baseIndex;
//            uint32_t baseVertex;
//            uint32_t indexCount;
//            glm::mat4 transform;
//            CustomShaderRendererComponentRenderPassOptions renderPassOptions;
//            std::pair<ShaderStorageBuffer*, ShaderStorageBuffer*> instanceBuffers = {nullptr, nullptr};
//        };

//        std::unordered_map<CustomShader*, std::vector<CustomShaderDrawCommand>> customShaderDeferredDrawCommandQueue;
//        std::unordered_map<CustomShader*, std::vector<CustomShaderDrawCommand>> customShaderForwardDrawCommandQueue;

        float findDrawInfoTextureIndex(UiDrawInfo& drawInfo, const Texture2D* texture) const;
        std::array<glm::vec4, 16> generateHBAOJitterNoise() const;
        size_t findCorrectLodIndex(const std::vector<float>& lodDistances, const glm::mat4& transform, size_t lodCount) const;
        void buildTransformBuffers();

        RenderingStats renderingStats;
        void resetRenderingStats();

    };

}
