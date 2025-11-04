#pragma once

namespace CgEngine {

    enum class ShaderDataType {
        Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    enum class DepthCompareOperator {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always,
    };

    enum class BlendingEquation {
        Add ,
        ReverseSubtract,
        Subtract,
        Min,
        Max
    };

    enum class BlendingFunction {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DestAlpha,
        OneMinusDestAlpha,
        DestColor,
        OneMinusDestColor
    };

    enum class TextureWrap {
        Repeat, Clamp, ClampBorder
    };

    enum class MipMapFiltering {
        Nearest, Bilinear, Trilinear, Anisotropic
    };

    enum class TextureBorderColor {
        OpaqueBlack, OpaqueWhite
    };

    enum class TextureFormat {
        R, RedFloat16, RedFloat32, RedGreenFloat16, RedGreenFloat32, RGB, RGBA, Float16A, Float32A, Float16, Float32
    };

    enum class AttachmentType {
        Depth, DepthStencil, RGBA8, RGBA16F, RGBA32F, RG8, RG16F, RG32F, R16F
    };

    enum class DepthAttachmentFormat {
        Depth32Float, Depth32FloatStencil8, Depth24Stencil8
    };

    enum class ShaderImageAccess {
        WriteOnly, ReadOnly, ReadWrite
    };

    enum class ShaderEnv {
        Engine, Custom
    };

    enum class VertexBufferUsage {
        Static, Dynamic
    };

    enum class IndexBufferDataType {
        UInt8, UInt16, UInt32
    };

    enum class ShaderStage {
        Undefined,
        Fragment,
        Compute,
        FragmentAndCompute
    };

    enum class DrawMode {
        Triangles, Patches, Lines
    };

}
