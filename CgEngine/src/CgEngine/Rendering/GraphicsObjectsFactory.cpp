#pragma once

#include "GraphicsObjectsFactory.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"
#include "Rendering/OpenGL/OpenGLVertexArrayObject.h"
#include "Rendering/OpenGL/OpenGLVertexBuffer.h"
#include "Rendering/OpenGL/OpenGLIndexBuffer.h"
#include "Rendering/OpenGL/OpenGLTexture2D.h"
#include "Rendering/OpenGL/OpenGLRenderPass.h"
#include "Rendering/OpenGL/OpenGLAttachment.h"
#include "Rendering/OpenGL/OpenGLFramebuffer.h"
#include "Rendering/OpenGL/OpenGLUniformBuffer.h"
#include "Rendering/OpenGL/OpenGLShaderStorageBuffer.h"
#include "Rendering/OpenGL/OpenGLImmutableShaderStorageBuffer.h"
#include "Rendering/OpenGL/OpenGLDescriptorSetLayout.h"
#include "Rendering/OpenGL/OpenGLDescriptorSet.h"
#include "Rendering/OpenGL/OpenGLTextureCube.h"
#include "Rendering/OpenGL/OpenGLComputePipeline.h"
#include "Rendering/OpenGL/OpenGLGraphicsPipeline.h"
#include "Rendering/Vulkan/VulkanRenderer.h"
#include "Rendering/Vulkan/VulkanVertexBuffer.h"
#include "Rendering/Vulkan/VulkanIndexBuffer.h"
#include "Rendering/Vulkan/VulkanVertexArrayObject.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Rendering/Vulkan/VulkanShaderStorageBuffer.h"
#include "Rendering/Vulkan/VulkanDescriptorSetLayout.h"
#include "Rendering/Vulkan/VulkanAttachment.h"
#include "Rendering/Vulkan/VulkanGraphicsPipeline.h"
#include "Rendering/Vulkan/VulkanImmutableShaderStorageBuffer.h"

namespace CgEngine {

    void GraphicsObjectsFactory::setGraphicsAPI(GraphicsAPI newApi) {
        api = newApi;
    }

    GraphicsAPI GraphicsObjectsFactory::getGraphicsAPI() {
        return api;
    }

    RendererBackendBase* GraphicsObjectsFactory::createRendererBackend() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanRenderer();
            case GraphicsAPI::OpenGL:
                return new OpenGLRenderer();
        }
    }

    IndexBuffer* GraphicsObjectsFactory::createIndexBuffer() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanIndexBuffer();
            case GraphicsAPI::OpenGL:
                return new OpenGLIndexBuffer();
        }
    }

    IndexBuffer* GraphicsObjectsFactory::createIndexBuffer(const void* indices, uint32_t indexCount, IndexBufferDataType type) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanIndexBuffer(indices, indexCount, type);
            case GraphicsAPI::OpenGL:
                return new OpenGLIndexBuffer(indices, indexCount, type);
        }
    }

    VertexBuffer* GraphicsObjectsFactory::createVertexBuffer(size_t size, VertexBufferUsage usage) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanVertexBuffer(size, usage);
            case GraphicsAPI::OpenGL:
                return new OpenGLVertexBuffer(size, usage);
        }
    }

    VertexBuffer* GraphicsObjectsFactory::createVertexBuffer(const void* data, size_t size, VertexBufferUsage usage) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanVertexBuffer(data, size, usage);
            case GraphicsAPI::OpenGL:
                return new OpenGLVertexBuffer(data, size, usage);
        }
    }

    VertexArrayObject* GraphicsObjectsFactory::createVertexArrayObject() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanVertexArrayObject();
            case GraphicsAPI::OpenGL:
                return new OpenGLVertexArrayObject();
        }
    }

    Texture2D* GraphicsObjectsFactory::createTexture2D(TextureFormat format, uint32_t width, uint32_t height, TextureWrap wrap, const void* data, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTexture2D(format, width, height, wrap, data, mipMapFiltering, borderColor);
        }
    }

    Texture2D* GraphicsObjectsFactory::createTexture2D(const std::filesystem::path& path, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTexture2D(path, srgb, wrap, mipMapFiltering, borderColor);
        }
    }

    Texture2D* GraphicsObjectsFactory::createTexture2D(const unsigned char* buffer, int bufferLen, bool srgb, TextureWrap wrap, MipMapFiltering mipMapFiltering, TextureBorderColor borderColor) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTexture2D(buffer, bufferLen, srgb, wrap, mipMapFiltering, borderColor);
        }
    }

    RenderPass* GraphicsObjectsFactory::createRenderPass() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLRenderPass();
        }
    }

    RenderPass* GraphicsObjectsFactory::createRenderPass(const RenderPassSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLRenderPass(spec);
        }
    }

    Attachment* GraphicsObjectsFactory::createAttachment() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanAttachment();
            case GraphicsAPI::OpenGL:
                return new OpenGLAttachment();
        }
    }

    Attachment* GraphicsObjectsFactory::createAttachment(const AttachmentSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanAttachment(spec);
            case GraphicsAPI::OpenGL:
                return new OpenGLAttachment(spec);
        }
    }

    Framebuffer* GraphicsObjectsFactory::createFramebuffer() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLFramebuffer();
        }
    }

    Framebuffer* GraphicsObjectsFactory::createFramebuffer(const FramebufferSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLFramebuffer(spec);
        }
    }

    UniformBuffer* GraphicsObjectsFactory::createUniformBuffer() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanUniformBuffer();
            case GraphicsAPI::OpenGL:
                return new OpenGLUniformBuffer();
        }
    }

    UniformBuffer* GraphicsObjectsFactory::createUniformBuffer(uint32_t size) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanUniformBuffer(size);
            case GraphicsAPI::OpenGL:
                return new OpenGLUniformBuffer(size);
        }
    }

    ShaderStorageBuffer* GraphicsObjectsFactory::createShaderStorageBuffer() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanShaderStorageBuffer();
            case GraphicsAPI::OpenGL:
                return new OpenGLShaderStorageBuffer();
        }
    }

    ShaderStorageBuffer* GraphicsObjectsFactory::createShaderStorageBuffer(uint32_t size) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanShaderStorageBuffer(size);
            case GraphicsAPI::OpenGL:
                return new OpenGLShaderStorageBuffer(size);
        }
    }

    ShaderStorageBuffer* GraphicsObjectsFactory::createShaderStorageBuffer(uint32_t size, const void* data) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanShaderStorageBuffer(size, data);
            case GraphicsAPI::OpenGL:
                return new OpenGLShaderStorageBuffer(size, data);
        }
    }

    ImmutableShaderStorageBuffer* GraphicsObjectsFactory::createImmutableShaderStorageBuffer() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanImmutableShaderStorageBuffer();
            case GraphicsAPI::OpenGL:
                return new OpenGLImmutableShaderStorageBuffer();
        }
    }

    ImmutableShaderStorageBuffer* GraphicsObjectsFactory::createImmutableShaderStorageBuffer(uint32_t size, const void* data) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanImmutableShaderStorageBuffer(size, data);
            case GraphicsAPI::OpenGL:
                return new OpenGLImmutableShaderStorageBuffer(size, data);
        }
    }

    DescriptorSetLayout* GraphicsObjectsFactory::createDescriptorSetLayout() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanDescriptorSetLayout();
            case GraphicsAPI::OpenGL:
                return new OpenGLDescriptorSetLayout();
        }
    }

    DescriptorSetLayout* GraphicsObjectsFactory::createDescriptorSetLayout(const DescriptorSetLayoutSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanDescriptorSetLayout(spec);
            case GraphicsAPI::OpenGL:
                return new OpenGLDescriptorSetLayout(spec);
        }
    }

    DescriptorSet* GraphicsObjectsFactory::createDescriptorSet() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLDescriptorSet();
        }
    }

    DescriptorSet* GraphicsObjectsFactory::createDescriptorSet(const DescriptorSetSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLDescriptorSet(spec);
        }
    }

    DescriptorSet* GraphicsObjectsFactory::createDescriptorSet(const DescriptorSetLayout* layout) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLDescriptorSet(layout);
        }
    }

    TextureCube* GraphicsObjectsFactory::createTextureCube() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTextureCube();
        }
    }

    TextureCube* GraphicsObjectsFactory::createTextureCube(TextureFormat format, uint32_t width, uint32_t height, MipMapFiltering mipMapFiltering) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTextureCube(format, width, height, mipMapFiltering);
        }
    }

    TextureCube* GraphicsObjectsFactory::createTextureCube(TextureFormat format, uint32_t width, uint32_t height, const void* data, MipMapFiltering mipMapFiltering) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLTextureCube(format, width, height, data, mipMapFiltering);
        }
    }

    ComputePipeline* GraphicsObjectsFactory::createComputePipeline() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLComputePipeline();
        }
    }

    ComputePipeline* GraphicsObjectsFactory::createComputePipeline(const ComputePipelineSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return nullptr;
            case GraphicsAPI::OpenGL:
                return new OpenGLComputePipeline(spec);
        }
    }

    GraphicsPipeline* GraphicsObjectsFactory::createGraphicsPipeline() {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanGraphicsPipeline();
            case GraphicsAPI::OpenGL:
                return new OpenGLGraphicsPipeline();
        }
    }

    GraphicsPipeline* GraphicsObjectsFactory::createGraphicsPipeline(const GraphicsPipelineSpecification& spec) {
        switch (api) {
            case GraphicsAPI::Vulkan:
                return new VulkanGraphicsPipeline(spec);
            case GraphicsAPI::OpenGL:
                return new OpenGLGraphicsPipeline(spec);
        }
    }

}
