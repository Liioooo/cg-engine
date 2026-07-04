#pragma once

namespace CgEngine {

    enum class ShaderDataType : uint8_t {
        Float   = 0,
        Float2  = 1,
        Float3  = 2,
        Float4  = 3,
        Int     = 4,
        Int2    = 5,
        Int3    = 6,
        Int4    = 7
    };

    enum class DepthCompareOperator : uint8_t {
        Never           = 0,
        Less            = 1,
        Equal           = 2,
        LessOrEqual     = 3,
        Greater         = 4,
        NotEqual        = 5,
        GreaterOrEqual  = 6,
        Always          = 7,
    };

    enum class BlendingEquation : uint8_t {
        Add             = 0,
        ReverseSubtract = 1,
        Subtract        = 2,
        Min             = 3,
        Max             = 4
    };

    enum class BlendingFunction {
        Zero                = 0,
        One                 = 1,
        SrcColor            = 2,
        OneMinusSrcColor    = 3,
        SrcAlpha            = 4,
        OneMinusSrcAlpha    = 5,
        DestAlpha           = 6,
        OneMinusDestAlpha   = 7,
        DestColor           = 8,
        OneMinusDestColor   = 9,
    };

    enum class TextureWrap : uint8_t {
        Repeat      = 0,
        Clamp       = 1,
        ClampBorder = 2
    };

    enum class MipMapFiltering : uint8_t {
        Nearest     = 0,
        Bilinear    = 1,
        Trilinear   = 2,
        Anisotropic = 3
    };

    enum class TextureBorderColor {
        OpaqueBlack, OpaqueWhite
    };

    enum class TextureFormat {
        R, RG, RGBA, RGBA_SRGB, RedFloat16, RedFloat32, RedGreenFloat16, RedGreenFloat32, Float16A, Float32A
    };

    enum class AttachmentType {
        Depth, DepthStencil, RGBA8, RGBA16F, RGBA32F, RG8, RG16F, RG32F, R16F
    };

    enum class DepthStencilAttachmentFormat {
        Depth32Float, Depth32FloatStencil8, Depth24Stencil8
    };

    enum class ShaderImageAccess {
        WriteOnly, ReadOnly, ReadWrite
    };

    enum class ShaderEnv {
        Engine, Custom
    };

    enum class VertexBufferUsage {
        Static, CPUDynamic, GPUDynamic
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

    enum class DrawMode : uint8_t {
        Triangles   = 0,
        Patches     = 1,
        Lines       = 2
    };

    enum class UIElementType : uint8_t {
        Circle  = 0,
        Rect    = 1,
        Text    = 2
    };

    enum class UIPosUnit : uint8_t {
        Pixel       = 0,
        VWPercent   = 1,
        VHPercent   = 2
    };

    enum class UIXAlignment : uint8_t {
        Left    = 0,
        Center  = 1,
        Right   = 2
    };

    enum class UIYAlignment : uint8_t {
        Top     = 0,
        Center  = 1,
        Bottom  = 2
    };
}
