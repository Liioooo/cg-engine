#pragma once

#include "Window.h"
#include "Rendering/RenderPass.h"
#include "Rendering/Framebuffer.h"
#include "Rendering/Texture2D.h"
#include "Rendering/TextureCube.h"
#include "Rendering/Material.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/DescriptorSet.h"
#include "Rendering/ComputePipeline.h"
#include "Rendering/GraphicsPipeline.h"

namespace CgEngine {

    class RendererBackendBase {
    public:
        virtual ~RendererBackendBase() = default;

        virtual void init(Window& window) = 0;
        virtual void shutdown() = 0;

        virtual void setFramebufferResized() = 0;

        virtual void beginFrame(const Window& window) = 0;
        virtual void endFrame(const Window& window) = 0;

        virtual void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) = 0;
        virtual void beginSwapChainRenderPass() = 0;
        virtual void endRenderPass() = 0;

        virtual void bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) = 0;
        virtual void bindComputePipeline(const ComputePipeline* computePipeline) = 0;
        virtual void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

        virtual void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) = 0;

        virtual void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) = 0;
        virtual void setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) = 0;

        virtual void transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) = 0;
        virtual void memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) = 0;

        virtual void renderUnitQuad() = 0;
        virtual void renderUnitCube() = 0;
        virtual void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) = 0;
        virtual void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) = 0;
        virtual void drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) = 0;

        virtual Texture2D* getWhiteTexture() = 0;
        virtual Texture2D* getBrdfLUTTexture() = 0;
        virtual TextureCube* getBlackCubeTexture() = 0;
        virtual std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath) = 0;

        virtual const std::vector<VertexBufferLayout> getUnitQuadVertexInputLayout() = 0;
        virtual const std::vector<VertexBufferLayout> getUnitCubeVertexInputLayout() = 0;
        virtual const RenderPass* getSwapChainRenderPass() = 0;

        virtual void beginImGuiFrame() = 0;
        virtual void renderImGuiFrame() = 0;
    };

}
