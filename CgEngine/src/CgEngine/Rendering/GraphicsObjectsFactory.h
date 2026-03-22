#pragma once

#include "GraphicsApi.h"
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {

    class RendererBackendBase;
    class IndexBuffer;
    class VertexBuffer;
    class VertexArrayObject;
    class Texture2D;
    class RenderPass;
    class Attachment;
    class Framebuffer;
    class UniformBuffer;
    class ShaderStorageBuffer;
    class ImmutableShaderStorageBuffer;
    class DescriptorSetLayout;
    class DescriptorSet;
    class TextureCube;
    class ComputePipeline;
    class GraphicsPipeline;
    class DynamicGraphicsPipeline;

    struct RenderPassSpecification;
    struct AttachmentSpecification;
    struct FramebufferSpecification;
    struct DescriptorSetLayoutSpecification;
    struct DescriptorSetSpecification;
    struct ComputePipelineSpecification;
    struct GraphicsPipelineSpecification;
    struct DynamicGraphicsPipelineSpecification;

    class GraphicsObjectsFactory {
    public:
        static void setGraphicsAPI(GraphicsAPI newApi);

        static RendererBackendBase* createRendererBackend();
        static IndexBuffer* createIndexBuffer();
        static IndexBuffer* createIndexBuffer(const void* indices, uint32_t indexCount, IndexBufferDataType type = IndexBufferDataType::UInt32);
        static VertexBuffer* createVertexBuffer(size_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
        static VertexBuffer* createVertexBuffer(const void* data, size_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
        static VertexArrayObject* createVertexArrayObject();
        static Texture2D* createTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic);
        static Texture2D* createTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering = MipMapFiltering::Anisotropic);
        static Texture2D* createTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap = TextureWrap::Repeat, MipMapFiltering mipMapFiltering= MipMapFiltering::Anisotropic);
        static RenderPass* createRenderPass();
        static RenderPass* createRenderPass(const RenderPassSpecification& spec);
        static Attachment* createAttachment();
        static Attachment* createAttachment(const AttachmentSpecification& spec);
        static Framebuffer* createFramebuffer();
        static Framebuffer* createFramebuffer(const FramebufferSpecification& spec);
        static UniformBuffer* createUniformBuffer();
        static UniformBuffer* createUniformBuffer(uint32_t size);
        static ShaderStorageBuffer* createShaderStorageBuffer();
        static ShaderStorageBuffer* createShaderStorageBuffer(uint32_t size);
        static ShaderStorageBuffer* createShaderStorageBuffer(uint32_t size, const void* data);
        static ImmutableShaderStorageBuffer* createImmutableShaderStorageBuffer();
        static ImmutableShaderStorageBuffer* createImmutableShaderStorageBuffer(uint32_t size, const void* data);
        static DescriptorSetLayout* createDescriptorSetLayout();
        static DescriptorSetLayout* createDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec);
        static DescriptorSet* createDescriptorSet();
        static DescriptorSet* createDescriptorSet(const DescriptorSetSpecification& spec);
        static TextureCube* createTextureCube();
        static TextureCube* createTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);
        static TextureCube* createTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering = MipMapFiltering::Bilinear);       ;
        static ComputePipeline* createComputePipeline();
        static ComputePipeline* createComputePipeline(const ComputePipelineSpecification& spec);
        static GraphicsPipeline* createGraphicsPipeline();
        static GraphicsPipeline* createGraphicsPipeline(const GraphicsPipelineSpecification& spec);
        static DynamicGraphicsPipeline* createDynamicGraphicsPipeline();
        static DynamicGraphicsPipeline* createDynamicGraphicsPipeline(const DynamicGraphicsPipelineSpecification& spec);

    private:
        static inline GraphicsAPI api = GraphicsAPI::OpenGL;
    };

}
