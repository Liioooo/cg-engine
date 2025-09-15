#pragma once

#include "RenderPass.h"
#include "Material.h"
#include "VertexArrayObject.h"
#include "ShaderStorageBuffer.h"
#include "Resources/ResRef.h"
#include "CustomShaders.h"
#include "Window.h"
#include "RendererBackendBase.h"

namespace CgEngine {

    struct LineDrawInfo {
        glm::vec3 from;
        glm::vec3 to;
        glm::vec3 color;
    };

    struct UiCircleVertex {
        glm::vec4 posUV;
        glm::vec4 lineColor;
        glm::vec4 fillColor;
        float width;
        float lineWidth;
        float textureIndex;
    };

    struct UiRectVertex {
        glm::vec4 posUV;
        glm::vec4 lineColor;
        glm::vec4 fillColor;
        glm::vec2 size;
        float lineWidth;
        float textureIndex;
    };

    struct UiTextVertex {
        glm::vec4 posUV;
        glm::vec4 color;
        float fontAtlasIndex;
    };

    class Renderer {
    public:
        static void init(Window& window);
        static void shutdown();

        static void beginFrame(const Window& window);
        static void endFrame(const Window& window);

        static void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer, const DescriptorSet* descriptorSet = nullptr);
        static void endRenderPass();

        static void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer);

        static void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex);
        static void setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount);

        static void renderUnitQuad(const Material& material);
        static void renderUnitCube(const Material& material);
//        static void renderLines(const std::vector<LineDrawInfo>& lines);
        static void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount);
//        static void executeCustomShaderDrawCommand(const VertexArrayObject& vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount, int tessellationPatchSize);

        static Texture2D* getWhiteTexture();
        static Texture2D* getBrdfLUTTexture();
        static TextureCube* getBlackCubeTexture();
        static std::pair<ResRef<TextureCube>, ResRef<TextureCube>> createEnvironmentMap(const std::string& hdriPath);

        static void beginImGuiFrame();
        static void renderImGuiFrame();

        static const uint32_t maxUiQuads = 5000;
        static const uint32_t maxUiIndices = maxUiQuads * 6;

        static const uint32_t maxTextureSlots = 16;

    private:
        static inline RendererBackendBase* backend;
    };

}
