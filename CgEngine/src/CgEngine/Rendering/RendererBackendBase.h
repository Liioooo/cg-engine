#pragma once

#include "Window.h"
#include "Rendering/RenderPass.h"
#include "Rendering/Framebuffer.h"
#include "Rendering/Texture2D.h"
#include "Rendering/TextureCube.h"
#include "Rendering/Material.h"
#include "Rendering/VertexArrayObject.h"
#include "Rendering/DescriptorSet.h"

namespace CgEngine {

    class RendererBackendBase {
    public:
        virtual ~RendererBackendBase() = default;

        virtual void init(Window& window) = 0;
        virtual void shutdown() = 0;

        virtual void beginFrame(const Window& window) = 0;
        virtual void endFrame(const Window& window) = 0;

        virtual void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer, const DescriptorSet* descriptorSet) = 0;
        virtual void endRenderPass() = 0;

        virtual void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) = 0;

        virtual void setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) = 0;

        virtual void renderUnitQuad(const Material& material) = 0;
        virtual void renderUnitCube(const Material& material) = 0;
        virtual void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) = 0;

        virtual Texture2D* getWhiteTexture() = 0;
        virtual Texture2D* getBrdfLUTTexture() = 0;
        virtual TextureCube* getBlackCubeTexture() = 0;
        virtual std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath) = 0;

        virtual void beginImGuiFrame() = 0;
        virtual void renderImGuiFrame() = 0;

    protected:
        static const uint32_t MAX_UI_QUADS = 5000;
        static const uint32_t MAX_UI_INDICES = MAX_UI_QUADS * 6;
    };

}
