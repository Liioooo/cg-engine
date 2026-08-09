#pragma once

#include "RenderPass.h"
#include "Camera.h"
#include "UniformBuffer.h"
#include "VertexArrayObject.h"
#include "ShaderStorageBuffer.h"
#include "Scene/Scene.h"
#include "Renderer.h"
#include "CameraFrustum.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "Ui/UiCanvas.h"
#include "Resources/Mesh.h"
#include "CustomPipeline.h"
#include "PipelineAttachmentInfo.h"

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
        float customShaderTimer = 0.0f;
        float skyboxTimer = 0.0f;
        float bloomTimer = 0.0f;
        float finalImageCompositeTimer = 0.0f;
        float uiCanvasTimer = 0.0f;
        float ui2DTimer = 0.0f;
        float toSrgbTimer = 0.0f;
    };

    class SceneRenderer {
    public:
        explicit SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight);
        ~SceneRenderer();

        void setActiveScene(Scene* scene);
        void setViewportSize(uint32_t width, uint32_t height);
        void beginScene(const Camera& camera, glm::mat4 cameraTransform, const SceneLightEnvironment& lightEnvironment, const SceneEnvironment& sceneEnvironment);
        void endScene();
        void submitMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, PBRMaterial* overrideMaterial, bool castShadows, bool enableCulling, const glm::mat4& transform, const std::vector<float>& lodDistances);
        void submitAnimatedMesh(MeshVertices* mesh, const std::vector<uint32_t>& meshNodes, PBRMaterial* overrideMaterial, bool castShadows, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, VertexArrayObject* skinnedVAO, const DescriptorSet* descriptorSet);
        void submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, bool enableCulling, const AABoundingBox* boundingBox, const glm::mat4& transform, CustomGraphicsPipeline* pipeline, uint32_t instanceCount, const std::vector<float>& lodDistances, const DescriptorSet* descriptorSet);
        void submitPhysicsColliderMesh(MeshVertices* mesh, const glm::mat4& transform);
        void submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, Mesh* mesh, const std::vector<uint32_t>& meshNodes, const glm::mat4& transform);
        void submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, const AABoundingBox& boundingBox, const glm::mat4& transform);
        void submitDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color);
        void submitUiCanvas2D(UiCanvas* uiCanvas, glm::mat4 finalTransform, uint32_t zIndex);

        const CameraFrustum& getCamaraFrustum() const;
        PipelineAttachmentInfo getGBufferAttachmentInfo() const;
        const DescriptorSetLayout* getCustomPipelineDescriptorSetLayout() const;
        const IndexBuffer* getUiIndexBuffer() const;
        const DescriptorSetLayout* getUiCanvasSampleDescriptorSetLayout() const;
        const DescriptorSetLayout* getUiDescriptorSetLayout() const;
        const DescriptorSetLayout* getUiTextDescriptorSetLayout() const;

        const DescriptorSetLayout* getEnvironmentMapDescriptorSetLayout() const;
        const DescriptorSetLayout* getPBRMaterialDescriptorSetLayout() const;
        const DescriptorSetLayout* getAnimatedMeshDescriptorSetLayout() const;

        const RenderingStats& getRenderingStats();

        struct CustomPipelineData {
            glm::mat4 transform;
        };

    private:
        static const uint32_t MAX_OBJECTS = 20000;
        static const uint32_t MAX_DEBUG_LINES = 1000;

        static const uint32_t MAX_BONES = 200;
        static const uint32_t MAX_ANIMATED_COMPONENTS = 512;

        Scene* activeScene;
        uint32_t viewportWidth;
        uint32_t viewportHeight;
        float invViewportWidth;
        float invViewportHeight;
        bool needsResize = true;
        bool activeRendering = false;

        DescriptorSetLayout* environmentMapDescriptorSetLayout;
        DescriptorSet* environmentMapDescriptorSetBlack;

        DescriptorSetLayout* dirShadowMapDescriptorSetLayout;
        RenderPass* dirShadowMapRenderPass;
        GraphicsPipeline* dirShadowMapPipeline;
        Attachment* dirShadowMaps;
        Framebuffer* dirShadowMapFramebuffer;
        ShaderStorageBuffer* dirShadowMapTransformsBuffer;
        DescriptorSet* dirShadowMapDescriptorSet;

        DescriptorSetLayout* pbrMaterialDescriptorSetLayout;

        DescriptorSetLayout* gBufferDescriptorSetLayout;
        RenderPass* gBufferRenderPass;
        GraphicsPipeline* gBufferPipeline;
        Attachment* gBufferAlbedoRoughnessAttachment;
        Attachment* gBufferEmissionMetallicAttachment;
        Attachment* gBufferWorldNormalsAttachment;
        Attachment* gBufferViewNormalsAttachment;
        Attachment* gBufferDepthAttachment;
        Framebuffer* gBufferFramebuffer;
        ShaderStorageBuffer* gBufferTransformsBuffer;
        DescriptorSet* gBufferDescriptorSet;

        glm::uvec3 hbaoWorkGroupSize;

        DescriptorSetLayout* hbaoDeinterleavingDescriptorSetLayout;
        RenderPass* hbaoDeinterleavingRenderPass;
        GraphicsPipeline* hbaoDeinterleavingPipeline;
        Attachment* hbaoDeinterleavingAttachment;
        std::array<Framebuffer*, 2> hbaoDeinterleavingFramebuffers;
        DescriptorSet* hbaoDeinterleavingDescriptorSet;

        DescriptorSetLayout* hbaoComputeDescriptorSetLayout;
        ComputePipeline* hbaoComputePipeline;
        Attachment* hbaoResult;
        DescriptorSet* hbaoComputeDescriptorSet;

        DescriptorSetLayout* hbaoReinterleavingDescriptorSetLayout;
        RenderPass* hbaoReinterleavingRenderPass;
        GraphicsPipeline* hbaoReinterleavingPipeline;
        Attachment* hbaoReinterleavingAttachment;
        Framebuffer* hbaoReinterleavingFramebuffer;
        DescriptorSet* hbaoReinterleavingDescriptorSet;

        struct alignas(8) HbaoBlurPushConstants {
            float sharpness;
            float _padding;
            glm::vec2 invResolutionDirection;
        };

        DescriptorSetLayout* hbaoBlurDescriptorSetLayout;
        RenderPass* hbaoBlurRenderPass0;
        RenderPass* hbaoBlurRenderPass1;
        GraphicsPipeline* hbaoBlurPipeline;
        Attachment* hbaoBlurAttachment0;
        Attachment* hbaoBlurAttachment1;
        Framebuffer* hbaoBlurFramebuffer0;
        Framebuffer* hbaoBlurFramebuffer1;
        DescriptorSet* hbaoBlurDescriptorSet0;
        DescriptorSet* hbaoBlurDescriptorSet1;

        DescriptorSetLayout* pbrDescriptorSetLayout;
        RenderPass* pbrRenderPass;
        GraphicsPipeline* pbrPipeline;
        Attachment* pbrColorAttachment;
        Framebuffer* pbrFramebuffer;
        DescriptorSet* pbrDescriptorSet;

        struct SkyboxPushConstants {
            float intensity;
            float lod;
        } skyboxPushConstants;

        RenderPass* afterPbrRenderPass;
        Framebuffer* afterPbrFramebuffer;

        DescriptorSetLayout* skyboxDescriptorSetLayout;
        DescriptorSet* skyboxDescriptorSet;
        GraphicsPipeline* skyboxPipeline;

        struct CollidersPushConstants {
            glm::vec3 color;
            int transformsOffset;
        };

        DescriptorSetLayout* boundingBoxDescriptorSetLayout;
        GraphicsPipeline* boundingBoxPipeline;
        ShaderStorageBuffer* boundingBoxTransformsBuffer;
        DescriptorSet* boundingBoxDescriptorSet;

        DescriptorSetLayout* physicsCollidersDescriptorSetLayout;
        GraphicsPipeline* physicsCollidersPipeline;
        ShaderStorageBuffer* physicsCollidersTransformsBuffer;
        DescriptorSet* physicsCollidersDescriptorSet;

        GraphicsPipeline* normalsDebugPipeline;

        DescriptorSetLayout* bloomDescriptorSetLayout;
        RenderPass* bloomDownSamplePass;
        RenderPass* bloomUpSamplePass;
        GraphicsPipeline* bloomDownsamplePipeline;
        GraphicsPipeline* bloomUpsamplePipeline;
        std::array<Attachment*, 7> bloomAttachments;
        std::array<Framebuffer*, 7> bloomDownsampleFramebuffers;
        std::array<Framebuffer*, 6> bloomUpsampleFramebuffers;
        std::array<DescriptorSet*, 8> bloomDescriptorSets;

        DescriptorSetLayout* finalImageCompositeDescriptorSetLayout;
        GraphicsPipeline* finalImageCompositePipeline;
        DescriptorSet* finalImageCompositeDescriptorSet;

        struct UiPushConstants {
            glm::mat4 projection;
        };
        DescriptorSetLayout* uiDescriptorSetLayout;
        DescriptorSetLayout* uiTextDescriptorSetLayout;
        GraphicsPipeline* uiCirclePipeline;
        GraphicsPipeline* uiRectPipeline;
        GraphicsPipeline* uiTextPipeline;

        GraphicsPipeline* ui2DPipeline;
        DescriptorSetLayout* ui2DDescriptorSetLayoutCameraBuffer;
        DescriptorSet* ui2DDescriptorSetCameraBuffer;
        DescriptorSetLayout* uiCanvasSampleDescriptorSetLayout;

        DescriptorSetLayout* debugLinesDescriptorSetLayout;
        GraphicsPipeline* debugLinesPipeline;
        DescriptorSet* debugLinesDescriptorSet;

        DescriptorSetLayout* customPipelineDescriptorSetLayout;
        DescriptorSet* customPipelineDescriptorSet;

        Attachment* finalImageLinearAttachment;
        RenderPass* finalImageLinearRenderPass;
        Framebuffer* finalImageLinearFramebuffer;

        DescriptorSetLayout* toSrgbDescriptorSetLayout;
        DescriptorSet* toSrgbDescriptorSet;
        GraphicsPipeline* toSrgbPipeline;

        struct SkinningPushConstants {
            int componentIndex;
        };
        DescriptorSetLayout* animatedMeshDescriptorSetLayout;
        DescriptorSetLayout* skinningDescriptorSetLayout;
        ShaderStorageBuffer* boneTransformsBuffer;
        ComputePipeline* skinningComputePipeline;
        DescriptorSet* skinningDescriptorSet;

        CameraFrustum cameraFrustum;

        void skinMeshes();
        void shadowMapPass();
        void gBufferPass();
        void hbaoDeinterleavingPass();
        void hbaoComputePass();
        void hbaoReinterleavingPass();
        void hbaoBlurPass();
        void pbrPass();
        void customShaderPass();
        void skyboxPass();
        void physicsCollidersPass();
        void boundingBoxPass();
        void normalsDebugPass();
        void debugLinesPass();
        void bloomPass();
        void finalImageCompositePass();
        void uiCanvasPass();
        void ui2DPass();
        void toSrbPass();

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
            uint32_t isOrtho;

            glm::vec4 float2Offsets[16];
            glm::vec4 jitters[16];
        } hbaoData;
        UniformBuffer* ubHBAOData;

        float hbaoSharpness = 1.0f;

        struct MeshKey {
            const VertexArrayObject* vao;
            const uint32_t submeshIndex;
            const uint32_t materialUuid;

            bool operator<(const MeshKey& other) const {
                if (vao < other.vao) return true;
                if (vao > other.vao) return false;
                if (submeshIndex < other.submeshIndex) return true;
                if (submeshIndex > other.submeshIndex) return false;
                return materialUuid < other.materialUuid;
            }
        };

        struct DrawCommand {
            VertexArrayObject* vao;
            const PBRMaterial* material;
            uint32_t baseIndex;
            uint32_t baseVertex;
            uint32_t indexCount;
            uint32_t instanceCount;
            int transformsBufferOffset;
        };

        struct DebugLineDrawInfo {
            glm::vec3 from;
            glm::vec3 to;
            glm::vec3 color;
        };

        std::map<MeshKey, DrawCommand> drawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> meshTransforms;

        std::map<MeshKey, DrawCommand> shadowMapDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> shadowMapMeshTransforms;

        std::map<MeshKey, DrawCommand> physicsCollidersDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> physicsCollidersMeshTransforms;

        std::map<MeshKey, DrawCommand> boundingBoxDrawCommandQueue;
        std::map<MeshKey, std::vector<glm::mat4>> boundingBoxMeshTransforms;

        std::vector<DebugLineDrawInfo> debugLinesDrawInfoQueue;

        struct SkinningInfo {
            const DescriptorSet* descriptorSet;
            VertexBuffer* skinnedVertexBuffer;
            uint32_t numVertices;
        };

        std::vector<SkinningInfo> skinningQueue;

        struct CurrentSceneEnvironment {
            const DescriptorSet* environmentMapDescriptorSet;
            float environmentIntensity;
            bool dirLightCastShadows;
        } currentSceneEnvironment;

        glm::mat4 uiProjectionMatrix;

        struct UiCanvasDrawCommand {
            Attachment* attachment;
            glm::mat4 projectionMatrix;
            glm::ivec2 pixelSize;
            std::vector<UiDrawCommand> drawCommands;
        };

        std::vector<UiCanvasDrawCommand> uiCanvasDrawCommandQueue;

        struct Ui2DDrawCommand {
            const DescriptorSet* sampleCanvasDescriptorSet;
            glm::mat4 finalTransform;
            uint32_t zIndex;
        };

        std::vector<Ui2DDrawCommand> ui2DDrawCommandQueue;

        struct CustomShaderDrawCommand {
            uint32_t instanceCount;
            VertexArrayObject* vao;
            const DescriptorSet* descriptorSet;
            uint32_t baseIndex;
            uint32_t baseVertex;
            uint32_t indexCount;
            glm::mat4 transform;
        };

        std::unordered_map<CustomGraphicsPipeline*, std::vector<CustomShaderDrawCommand>> customShaderDrawCommandQueue;

        IndexBuffer* uiIndexBuffer;

        VertexArrayObject* debugLinesVAO;

        std::array<glm::vec4, 16> generateHBAOJitterNoise() const;
        size_t findCorrectLodIndex(const std::vector<float>& lodDistances, const glm::mat4& transform, size_t lodCount) const;
        void buildTransformBuffers();
        void fillDebugLinesVertexBuffer();

        RenderingStats renderingStats;
        void resetRenderingStats();

    };

}
