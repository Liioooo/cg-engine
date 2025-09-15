#pragma once

#include "Rendering/RendererBackendBase.h"
#include "OpenGLVertexArrayObject.h"
#include "Rendering/Enums.h"
#include "OpenGLTexture2D.h"
#include "OpenGLRenderPass.h"
#include "OpenGLTextureCube.h"

namespace CgEngine {

    class OpenGLRenderer : public RendererBackendBase {
    public:
        void init(Window& window) override;
        void shutdown() override;

        void beginFrame(const Window& window) override;
        void endFrame(const Window& window) override;

        void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer, const DescriptorSet* descriptorSet) override;
        void endRenderPass() override;

        void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;

        void setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) override;

        void renderUnitQuad(const Material& material) override;
        void renderUnitCube(const Material& material) override;
        void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) override;

        Texture2D* getWhiteTexture() override;
        Texture2D* getBrdfLUTTexture() override;
        TextureCube* getBlackCubeTexture() override;
        std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath) override;

        void beginImGuiFrame() override;
        void renderImGuiFrame() override;

    private:
        const OpenGLRenderPass* currentRenderPass;
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

        OpenGLTexture2D whiteTexture;
        OpenGLTexture2D brdfLUT;
        OpenGLTextureCube blackCubeTexture;

        OpenGLVertexArrayObject quadVAO;
        OpenGLVertexArrayObject unitCubeVAO;
        OpenGLVertexArrayObject linesVAO;

        OpenGLIndexBuffer uiIndexBuffer;
        OpenGLVertexArrayObject uiCircleVAO;
        OpenGLVertexArrayObject uiRectVAO;
        OpenGLVertexArrayObject uiTextVAO;

        static void initImGui(Window& window);
        static void shutdownImGui();
        static void debugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam);
    };

}
