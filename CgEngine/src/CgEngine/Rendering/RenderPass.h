#pragma once

#include "Shader.h"
#include "Framebuffer.h"

namespace CgEngine {

    enum class DepthCompareOperator {
        Never =             0x0200, // GL_NEVER
        Less =              0x0201, // GL_LESS
        Equal =             0x0202, // GL_EQUAL
        LessOrEqual =       0x0203, // GL_LEQUAL
        Greater =           0x0204, // GL_GREATER
        NotEqual =          0x0205, // GL_NOTEQUAL
        GreaterOrEqual =    0x0206, // GL_GEQUAL
        Always =            0x0207, // GL_ALWAYS
    };

    enum class BlendingEquation {
         Add =              0x8006, // GL_FUNC_ADD
         ReverseSubtract =  0x800B, // GL_FUNC_REVERSE_SUBTRACT
         Subtract =         0x800A, // GL_FUNC_SUBTRACT
         Min =              0x8007, // GL_MIN
         Max =              0x8008 // GL_MAX
    };

    enum class BlendingFunction {
        Zero =              0, // GL_ZERO
        One =               1, // GL_ONE
        SrcColor =          0x0300, // GL_SRC_COLOR
        OneMinusSrcColor =  0x0301, // GL_ONE_MINUS_SRC_COLOR
        SrcAlpha =          0x0302, // GL_SRC_ALPHA
        OneMinusSrcAlpha =  0x0303, // GL_ONE_MINUS_SRC_ALPHA
        DestAlpha =         0x0304, // GL_DST_ALPHA
        OneMinusDestAlpha = 0x0305, // GL_ONE_MINUS_DST_ALPHA
        DestColor =         0x0306, // GL_DST_COLOR
        OneMinusDestColor = 0x0307, // GL_ONE_MINUS_DST_COLOR
    };

    struct RenderPassSpecification {
        Shader* shader = nullptr;
        DepthCompareOperator depthCompareOperator = DepthCompareOperator::Less;
        Framebuffer* framebuffer = nullptr;
        bool clearDepthBuffer = true;
        bool clearColorBuffer = true;
        bool clearStencilBuffer = false;
        bool backfaceCulling = true;
        bool frontfaceCulling = false;
        bool depthTest = true;
        bool depthWrite = true;
        bool wireframe = false;
        bool useBlending = false;
        BlendingEquation blendingEquation = BlendingEquation::Add;
        BlendingFunction srcBlendingFunction = BlendingFunction::SrcAlpha;
        BlendingFunction destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
    };

    class RenderPass {
    public:
        explicit RenderPass(RenderPassSpecification spec);
        ~RenderPass();

        const RenderPassSpecification& getSpecification();

    private:
        RenderPassSpecification specification;
    };

}
