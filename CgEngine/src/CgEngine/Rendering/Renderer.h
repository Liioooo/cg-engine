#pragma once

#include "RenderPass.h"
#include "VertexArrayObject.h"
#include "Resources/ResRef.h"
#include "Window.h"
#include "RendererBackendBase.h"
#include "ComputePipeline.h"
#include "GraphicsPipeline.h"
#include "PipelineAttachmentInfo.h"

namespace CgEngine {

    class OpenGLRenderer;
    class VulkanRenderer;

    class Renderer {
    public:
        static void init(Window& window);
        static void shutdown();

        static void setFramebufferResized();

        static bool beginFrame(const Window& window);
        static void endFrame(const Window& window);

        static void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer);
        static void beginSwapChainRenderPass();
        static void endRenderPass();

        static void beginDynamicRendering(const DynamicRenderingInfo& renderingInfo);
        static void endDynamicRendering();

        static void bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline);
        static void bindComputePipeline(const ComputePipeline* computePipeline);
        static void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        static void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer);

        static void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex);
        static void setPushConstants(const void* data, size_t size);

        static void injectBarriersForDescriptorSet(const DescriptorSet* descriptorSet);

        static void transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition);
        static void memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer);
        static void memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment);

        static void renderUnitQuad();
        static void renderUnitCube();
        static void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount);
        static void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex);
        static void drawArrays(const VertexArrayObject* vao, uint32_t vertexCount);

        static Texture2D* getWhiteTexture();
        static Texture2D* getBrdfLUTTexture();
        static TextureCube* getBlackCubeTexture();
        static std::pair<ResRef<TextureCube>, ResRef<TextureCube>> createEnvironmentMap(const std::string& hdriPath);

        static const std::vector<VertexBufferLayout> getUnitQuadVertexInputLayout();
        static const std::vector<VertexBufferLayout> getUnitCubeVertexInputLayout();
        static const PipelineAttachmentInfo getSwapChainAttachmentInfo();

        static void beginImGuiFrame();
        static void renderImGuiFrame();

        static OpenGLRenderer* getOpenGLBackend();
        static VulkanRenderer* getVulkanBackend();

    private:
        static inline RendererBackendBase* backend;
    };

}
