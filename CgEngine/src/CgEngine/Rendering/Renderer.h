#pragma once

#include "RenderPass.h"
#include "Material.h"
#include "VertexArrayObject.h"
#include "ShaderStorageBuffer.h"

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
        static void init();
        static void shutdown();
        static void beginRenderPass(RenderPass& renderPass);
        static void endRenderPass();
        static void renderUnitQuad(const Material& material);
        static void renderUnitCube(const Material& material);
        static void renderLines(const std::vector<LineDrawInfo>& lines);
        static void executeDrawCommand(const VertexArrayObject& vao, const Material& material, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, const std::vector<glm::mat4>& transforms, uint32_t instanceCount);
        static void renderUiCircles(const std::vector<UiCircleVertex>& vertices, uint32_t indexCount);
        static void renderUiRects(const std::vector<UiRectVertex>& vertices, uint32_t indexCount);
        static void renderUiText(const std::vector<UiTextVertex>& vertices, uint32_t indexCount);

        static Texture2D& getWhiteTexture();
        static Texture2D& getBrdfLUTTexture();
        static TextureCube& getBlackCubeTexture();
        static Material* getDefaultPBRMaterial();
        static std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath);

        static const uint32_t maxUiQuads = 5000;
        static const uint32_t maxUiIndices = maxUiQuads * 6;

        static const uint32_t maxTextureSlots = 16;

    private:
        static inline RenderPass* currentRenderPass;
        static inline bool isWireframe;
        static inline bool isBackFaceCulling;
        static inline bool isFrontFaceCulling;
        static inline DepthCompareOperator depthCompareOperator;
        static inline bool depthTest;
        static inline bool depthWrite;
        static inline bool useBlending;
        static inline BlendingEquation blendingEquation;
        static inline BlendingFunction srcBlendingFunction;
        static inline BlendingFunction destBlendingFunction;

        static inline Texture2D* whiteTexture;
        static inline Texture2D* brdfLUT;
        static inline TextureCube* blackCubeTexture;

        static inline VertexArrayObject* quadVAO;
        static inline VertexArrayObject* unitCubeVAO;
        static inline VertexArrayObject* linesVAO;

        static inline VertexArrayObject* uiCircleVAO;
        static inline VertexArrayObject* uiRectVAO;
        static inline VertexArrayObject* uiTextVAO;

        static inline ShaderStorageBuffer* transformsBuffer;
    };

}
