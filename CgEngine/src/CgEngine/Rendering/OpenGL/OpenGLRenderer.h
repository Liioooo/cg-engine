#pragma once

#include "Rendering/RendererBackendBase.h"
#include "OpenGLVertexArrayObject.h"
#include "CgEngineSharedUtils/Enums.h"
#include "OpenGLTexture2D.h"
#include "OpenGLRenderPass.h"
#include "OpenGLTextureCube.h"
#include "OpenGLComputePipeline.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLGraphicsPipeline.h"

namespace CgEngine {

    class OpenGLRenderer : public RendererBackendBase {
    public:
        void init(Window& window) override;
        void shutdown() override;

        void setFramebufferResized() override;

        void beginFrame(const Window& window) override;
        void endFrame(const Window& window) override;

        void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;
        void beginSwapChainRenderPass() override;
        void endRenderPass() override;

        void beginDynamicRendering(const DynamicRenderingInfo& renderingInfo) override;
        void endDynamicRendering() override;

        void bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) override;
        void bindComputePipeline(const ComputePipeline* computePipeline) override;
        void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;

        void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) override;
        void setPushConstants(const void* data, size_t size) override;

        void injectBarriersForDescriptorSet(const DescriptorSet *descriptorSet) override;

        void transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) override;
        void memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) override;
        void memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment) override;

        void renderUnitQuad() override;
        void renderUnitCube() override;
        void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) override;
        void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) override;
        void drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) override;

        Texture2D* getWhiteTexture() override;
        Texture2D* getBrdfLUTTexture() override;
        TextureCube* getBlackCubeTexture() override;
        std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath) override;

        const std::vector<VertexBufferLayout> getUnitQuadVertexInputLayout() override;
        const std::vector<VertexBufferLayout> getUnitCubeVertexInputLayout() override;
        PipelineAttachmentInfo getSwapChainAttachmentInfo() override;

        void beginImGuiFrame() override;
        void renderImGuiFrame() override;

        GraphicsAPI getGraphicsAPI() const override { return GraphicsAPI::OpenGL; }

    private:
        const OpenGLRenderPass* currentRenderPass;
        bool currentlyDynamicRendering = false;
        uint32_t currentPipelineHandle = ~0;
        unsigned int drawMode;
        bool isWireframe;
        bool isBackFaceCulling;
        bool isFrontFaceCulling;
        DepthCompareOperator depthCompareOperator;
        bool depthTest;
        bool depthWrite;
        bool useBlending;
        BlendingEquation blendingEquation;
        BlendingFunction srcBlendingFunction;
        BlendingFunction destBlendingFunction;
        int tessellationPatchSize;

        unsigned int dynamicRenderingFramebufferHandle = ~0;

        OpenGLRenderPass swapChainRenderPass;
        OpenGLFramebuffer swapChainFramebuffer;
        bool framebufferResized = false;

        OpenGLTexture2D whiteTexture;
        OpenGLTexture2D brdfLUT;
        OpenGLTextureCube blackCubeTexture;

        OpenGLComputePipeline computeEnvironmentMapSphereToCube;
        OpenGLComputePipeline computeEnvironmentMapPrefilterMap;
        OpenGLComputePipeline computeEnvironmentMapIrradianceMap;

        OpenGLVertexArrayObject quadVAO;
        OpenGLVertexArrayObject unitCubeVAO;

        unsigned int pushConstantsBuffer = ~0;

        void initImGui(Window& window);
        void shutdownImGui();
        static void debugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam);
    };

}
