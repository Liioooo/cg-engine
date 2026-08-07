#include "SceneRenderer.h"
#include "Asserts.h"
#include "Application.h"
#include "GPUTimer.h"
#include "GPUDebugGroup.h"
#include "GraphicsObjectsFactory.h"
#include "Helpers.h"
#include "Ui/UIVertexBufferLayouts.h"

namespace CgEngine {
    SceneRenderer::SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight), invViewportWidth(1.0f / static_cast<float>(viewportWidth)), invViewportHeight(1.0f / static_cast<float>(viewportHeight)) {
        ubCameraData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBCameraData));
        ubLightData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBLightData));
        ubDirShadowData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBDirShadowData));
        ubScreenData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBScreenData));
        ubHBAOData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBHBAOData));

        {
            DescriptorSetLayoutSpecification pbrMaterialDescriptorSetLayoutSpec{};
            pbrMaterialDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {
                {0, DescriptorSetLayoutBindingUsage::Fragment},
                {1, DescriptorSetLayoutBindingUsage::Fragment},
                {2, DescriptorSetLayoutBindingUsage::Fragment},
                {3, DescriptorSetLayoutBindingUsage::Fragment},
                {4, DescriptorSetLayoutBindingUsage::Fragment}
            };
            pbrMaterialDescriptorSetLayoutSpec.uboBindingPoints = {{5, DescriptorSetLayoutBindingUsage::Fragment}};
            pbrMaterialDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(pbrMaterialDescriptorSetLayoutSpec);
        }
        {
            DescriptorSetLayoutSpecification environmentMapDescriptorSetLayoutSpec{};
            environmentMapDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {
                {5, DescriptorSetLayoutBindingUsage::Fragment},
                {6, DescriptorSetLayoutBindingUsage::Fragment}
            };

            environmentMapDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(environmentMapDescriptorSetLayoutSpec);

            DescriptorSetSpecification environmentMapDescriptorSetSpec{};
            environmentMapDescriptorSetSpec.layout = environmentMapDescriptorSetLayout;
            environmentMapDescriptorSetSpec.textureCubeBindings = {
                {5, Renderer::getBlackCubeTexture()},
                {6, Renderer::getBlackCubeTexture()}
            };
            environmentMapDescriptorSetBlack = GraphicsObjectsFactory::createDescriptorSet(environmentMapDescriptorSetSpec);
        }
        {
            ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

            AttachmentSpecification dirShadowMapAttachmentSpec{};
            dirShadowMapAttachmentSpec.height = applicationOptions.shadowMapResolution;
            dirShadowMapAttachmentSpec.width = applicationOptions.shadowMapResolution;
            dirShadowMapAttachmentSpec.type = AttachmentType::Depth;
            dirShadowMapAttachmentSpec.usableAsTexture = true;
            dirShadowMapAttachmentSpec.textureWrap = TextureWrap::ClampBorder;
            dirShadowMapAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
            dirShadowMapAttachmentSpec.layerCount = 4;
            dirShadowMapAttachmentSpec.textureBorderColor = TextureBorderColor::OpaqueWhite;

            dirShadowMaps = GraphicsObjectsFactory::createAttachment(dirShadowMapAttachmentSpec);

            RenderPassSpecification shadowMapRenderPassSpec{};
            shadowMapRenderPassSpec.clearColorAttachments = false;
            shadowMapRenderPassSpec.clearDepthStencilAttachment = true;
            shadowMapRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

            dirShadowMapRenderPass = GraphicsObjectsFactory::createRenderPass(shadowMapRenderPassSpec);

            DescriptorSetLayoutSpecification dirShadowMapDescriptorSetLayoutSpec{};
            dirShadowMapDescriptorSetLayoutSpec.uboBindingPoints = {{2, DescriptorSetLayoutBindingUsage::Geometry}};
            dirShadowMapDescriptorSetLayoutSpec.ssboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}};
            dirShadowMapDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(dirShadowMapDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification dirShadowMapPipelineSpec{};
            dirShadowMapPipelineSpec.engineShaderName = "dirShadowMap";
            dirShadowMapPipelineSpec.frontfaceCulling = false;
            dirShadowMapPipelineSpec.backfaceCulling = true;
            dirShadowMapPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;
            dirShadowMapPipelineSpec.descriptorSetLayouts = {dirShadowMapDescriptorSetLayout};
            dirShadowMapPipelineSpec.colorAttachments = {};
            dirShadowMapPipelineSpec.hasDepthStencilAttachment = true;
            dirShadowMapPipelineSpec.depthAttachmentFormat = dirShadowMaps->getDepthStencilAttachmentFormat();
            dirShadowMapPipelineSpec.usesPushConstants = true;
            dirShadowMapPipelineSpec.pushConstantsSize = sizeof(int);

            dirShadowMapPipeline = GraphicsObjectsFactory::createGraphicsPipeline(dirShadowMapPipelineSpec);

            FramebufferSpecification shadowMapFramebufferSpec{};
            shadowMapFramebufferSpec.height = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.width = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.depthStencilAttachment.attachment = dirShadowMaps;

            dirShadowMapFramebuffer = GraphicsObjectsFactory::createFramebuffer(shadowMapFramebufferSpec);

            dirShadowMapTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification dirShadowMapDescriptorSetSpec{};
            dirShadowMapDescriptorSetSpec.layout = dirShadowMapDescriptorSetLayout;
            dirShadowMapDescriptorSetSpec.uboBindings = {
                {2, ubDirShadowData}
            };
            dirShadowMapDescriptorSetSpec.ssboBindings = {
                {0, dirShadowMapTransformsBuffer}
            };

            dirShadowMapDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(dirShadowMapDescriptorSetSpec);
        }
        {
            AttachmentSpecification gBufferAttachmentSpec{};
            gBufferAttachmentSpec.width = viewportWidth;
            gBufferAttachmentSpec.height = viewportHeight;
            gBufferAttachmentSpec.type = AttachmentType::RGBA16F;
            gBufferAttachmentSpec.usableAsTexture = true;
            gBufferAttachmentSpec.textureWrap = TextureWrap::Clamp;
            gBufferAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
            gBufferAttachmentSpec.layerCount = 1;

            gBufferAlbedoRoughnessAttachment = GraphicsObjectsFactory::createAttachment(gBufferAttachmentSpec);
            gBufferEmissionMetallicAttachment = GraphicsObjectsFactory::createAttachment(gBufferAttachmentSpec);
            gBufferWorldNormalsAttachment = GraphicsObjectsFactory::createAttachment(gBufferAttachmentSpec);
            gBufferViewNormalsAttachment = GraphicsObjectsFactory::createAttachment(gBufferAttachmentSpec);

            AttachmentSpecification gBufferDepthAttachmentSpec{};
            gBufferDepthAttachmentSpec.width = viewportWidth;
            gBufferDepthAttachmentSpec.height = viewportHeight;
            gBufferDepthAttachmentSpec.type = AttachmentType::Depth;
            gBufferDepthAttachmentSpec.usableAsTexture = true;
            gBufferDepthAttachmentSpec.textureWrap = TextureWrap::Clamp;
            gBufferDepthAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
            gBufferDepthAttachmentSpec.layerCount = 1;

            gBufferDepthAttachment = GraphicsObjectsFactory::createAttachment(gBufferDepthAttachmentSpec);

            RenderPassSpecification gBufferRenderPassSpec{};
            gBufferRenderPassSpec.clearColorAttachments = true;
            gBufferRenderPassSpec.clearDepthStencilAttachment = true;
            gBufferRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

            gBufferRenderPass = GraphicsObjectsFactory::createRenderPass(gBufferRenderPassSpec);

            DescriptorSetLayoutSpecification gBufferDescriptorSetLayoutSpec{};
            gBufferDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex | DescriptorSetLayoutBindingUsage::Geometry}};
            gBufferDescriptorSetLayoutSpec.ssboBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Vertex}};
            gBufferDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(gBufferDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification gBufferPipelineSpec{};
            gBufferPipelineSpec.engineShaderName = "gBuffer";
            gBufferPipelineSpec.depthCompareOperator = DepthCompareOperator::Less;
            gBufferPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;
            gBufferPipelineSpec.descriptorSetLayouts = {gBufferDescriptorSetLayout, pbrMaterialDescriptorSetLayout};
            gBufferPipelineSpec.colorAttachments = {
                AttachmentType::RGBA16F,
                AttachmentType::RGBA16F,
                AttachmentType::RGBA16F,
                AttachmentType::RGBA16F
            };
            gBufferPipelineSpec.hasDepthStencilAttachment = true;
            gBufferPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            gBufferPipelineSpec.usesPushConstants = true;
            gBufferPipelineSpec.pushConstantsSize = sizeof(int);

            gBufferPipeline = GraphicsObjectsFactory::createGraphicsPipeline(gBufferPipelineSpec);

            FramebufferSpecification gBufferFramebufferSpec{};
            gBufferFramebufferSpec.height = viewportHeight;
            gBufferFramebufferSpec.width = viewportWidth;
            gBufferFramebufferSpec.colorAttachments = {
                {gBufferAlbedoRoughnessAttachment},
                {gBufferEmissionMetallicAttachment},
                {gBufferWorldNormalsAttachment},
                {gBufferViewNormalsAttachment}
            };
            gBufferFramebufferSpec.depthStencilAttachment.attachment = gBufferDepthAttachment;
            gBufferFramebufferSpec.depthStencilAttachment.allLayers = true;

            gBufferFramebuffer = GraphicsObjectsFactory::createFramebuffer(gBufferFramebufferSpec);

            gBufferTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification gBufferDescriptorSetSpec{};
            gBufferDescriptorSetSpec.layout = gBufferDescriptorSetLayout;
            gBufferDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData}
            };
            gBufferDescriptorSetSpec.ssboBindings = {
                    {1, gBufferTransformsBuffer}
            };

            gBufferDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(gBufferDescriptorSetSpec);
        }
        {
            DescriptorSetLayoutSpecification customPipelineDescriptorSetLayoutSpec{};
            customPipelineDescriptorSetLayoutSpec.uboBindingPoints = {
                {0, DescriptorSetLayoutBindingUsage::AllGraphics},
                {3, DescriptorSetLayoutBindingUsage::AllGraphics}
            };

            customPipelineDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(customPipelineDescriptorSetLayoutSpec);

            DescriptorSetSpecification customPipelineDescriptorSetSpec{};
            customPipelineDescriptorSetSpec.layout = customPipelineDescriptorSetLayout;
            customPipelineDescriptorSetSpec.uboBindings = {
                {0, ubCameraData},
                {3, ubScreenData}
            };

            customPipelineDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(customPipelineDescriptorSetSpec);
        }
        {
            glm::uvec2 quarterSize = (glm::uvec2(viewportWidth, viewportHeight) + 3u) / 4u;

            AttachmentSpecification hbaoDeinterleavingAttachmentSpec{};
            hbaoDeinterleavingAttachmentSpec.width = quarterSize.x;
            hbaoDeinterleavingAttachmentSpec.height = quarterSize.y;
            hbaoDeinterleavingAttachmentSpec.type = AttachmentType::R16F;
            hbaoDeinterleavingAttachmentSpec.layerCount = 16;
            hbaoDeinterleavingAttachmentSpec.usableAsTexture = true;
            hbaoDeinterleavingAttachmentSpec.textureWrap = TextureWrap::Clamp;
            hbaoDeinterleavingAttachmentSpec.mipMapFiltering = MipMapFiltering::Nearest;

            hbaoDeinterleavingAttachment = GraphicsObjectsFactory::createAttachment(hbaoDeinterleavingAttachmentSpec);

            RenderPassSpecification hbaoDeinterleavingRenderPassSpec{};
            hbaoDeinterleavingRenderPassSpec.clearDepthStencilAttachment = false;
            hbaoDeinterleavingRenderPassSpec.clearColorAttachments = true;
            hbaoDeinterleavingRenderPassSpec.clearColor = {1.0f, 1.0f, 1.0f, 1.0f};

            hbaoDeinterleavingRenderPass = GraphicsObjectsFactory::createRenderPass(hbaoDeinterleavingRenderPassSpec);

            DescriptorSetLayoutSpecification hbaoDeinterleavingDescriptorSetLayoutSpec{};
            hbaoDeinterleavingDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Fragment}};
            hbaoDeinterleavingDescriptorSetLayoutSpec.uboBindingPoints = {
                {0, DescriptorSetLayoutBindingUsage::Fragment},
                {3, DescriptorSetLayoutBindingUsage::Fragment}
            };
            hbaoDeinterleavingDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(hbaoDeinterleavingDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification hbaoDeinterleavingPipelineSpec{};
            hbaoDeinterleavingPipelineSpec.descriptorSetLayouts = {hbaoDeinterleavingDescriptorSetLayout};
            hbaoDeinterleavingPipelineSpec.engineShaderName = "hbaoDeinterleaving";
            hbaoDeinterleavingPipelineSpec.depthWrite = false;
            hbaoDeinterleavingPipelineSpec.depthTest = false;
            hbaoDeinterleavingPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            hbaoDeinterleavingPipelineSpec.colorAttachments = {
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F,
                AttachmentType::R16F
            };
            hbaoDeinterleavingPipelineSpec.hasDepthStencilAttachment = false;
            hbaoDeinterleavingPipelineSpec.usesPushConstants = true;
            hbaoDeinterleavingPipelineSpec.pushConstantsSize = sizeof(int);

            hbaoDeinterleavingPipeline = GraphicsObjectsFactory::createGraphicsPipeline(hbaoDeinterleavingPipelineSpec);

            FramebufferSpecification hbaoDeinterleavingFramebufferSpec0{};
            hbaoDeinterleavingFramebufferSpec0.width = quarterSize.x;
            hbaoDeinterleavingFramebufferSpec0.height = quarterSize.y;
            hbaoDeinterleavingFramebufferSpec0.colorAttachments = {
                {hbaoDeinterleavingAttachment, 0, false},
                {hbaoDeinterleavingAttachment, 1, false},
                {hbaoDeinterleavingAttachment, 2, false},
                {hbaoDeinterleavingAttachment, 3, false},
                {hbaoDeinterleavingAttachment, 4, false},
                {hbaoDeinterleavingAttachment, 5, false},
                {hbaoDeinterleavingAttachment, 6, false},
                {hbaoDeinterleavingAttachment, 7, false}
            };

            hbaoDeinterleavingFramebuffers[0] = GraphicsObjectsFactory::createFramebuffer(hbaoDeinterleavingFramebufferSpec0);

            FramebufferSpecification hbaoDeinterleavingFramebufferSpec1{};
            hbaoDeinterleavingFramebufferSpec1.width = quarterSize.x;
            hbaoDeinterleavingFramebufferSpec1.height = quarterSize.y;
            hbaoDeinterleavingFramebufferSpec1.colorAttachments = {
                    {hbaoDeinterleavingAttachment, 8, false},
                    {hbaoDeinterleavingAttachment, 9, false},
                    {hbaoDeinterleavingAttachment, 10, false},
                    {hbaoDeinterleavingAttachment, 11, false},
                    {hbaoDeinterleavingAttachment, 12, false},
                    {hbaoDeinterleavingAttachment, 13, false},
                    {hbaoDeinterleavingAttachment, 14, false},
                    {hbaoDeinterleavingAttachment, 15, false}
            };

            hbaoDeinterleavingFramebuffers[1] = GraphicsObjectsFactory::createFramebuffer(hbaoDeinterleavingFramebufferSpec1);

            DescriptorSetSpecification hbaoDeinterleavingDescriptorSetSpec{};
            hbaoDeinterleavingDescriptorSetSpec.layout = hbaoDeinterleavingDescriptorSetLayout;
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings.resize(1);
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].attachment = gBufferDepthAttachment;
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 1;
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            hbaoDeinterleavingDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData},
                    {3, ubScreenData}
            };

            hbaoDeinterleavingDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(hbaoDeinterleavingDescriptorSetSpec);

            DescriptorSetLayoutSpecification hbaoComputeDescriptorSetLayoutSpec{};
            hbaoComputeDescriptorSetLayoutSpec.uboBindingPoints = {
                {3, DescriptorSetLayoutBindingUsage::Compute},
                {4, DescriptorSetLayoutBindingUsage::Compute}
            };
            hbaoComputeDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {
                {0, DescriptorSetLayoutBindingUsage::Compute},
                {1, DescriptorSetLayoutBindingUsage::Compute}
            };
            hbaoComputeDescriptorSetLayoutSpec.imageBindingPoints = {{2, DescriptorSetLayoutBindingUsage::Compute}};
            hbaoComputeDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(hbaoComputeDescriptorSetLayoutSpec);

            ComputePipelineSpecification hbaoComputePipelineSpec{};
            hbaoComputePipelineSpec.descriptorSetLayouts = {hbaoComputeDescriptorSetLayout};
            hbaoComputePipelineSpec.engineShaderName = "hbao";

            hbaoComputePipeline = GraphicsObjectsFactory::createComputePipeline(hbaoComputePipelineSpec);

            for (int i = 0; i < 16; i++) {
                hbaoData.float2Offsets[i] = glm::vec4((float)(i % 4) + 0.5f, (float)(i / 4.0f) + 0.5f, 0.0f, 1.f);
            }
            std::memcpy(hbaoData.jitters, generateHBAOJitterNoise().data(), sizeof(glm::vec4) * 16);

            AttachmentSpecification hbaoResultSpec{};
            hbaoResultSpec.width = quarterSize.x;
            hbaoResultSpec.height = quarterSize.y;
            hbaoResultSpec.type = AttachmentType::RG16F;
            hbaoResultSpec.layerCount = 16;
            hbaoResultSpec.usableAsTexture = true;
            hbaoResultSpec.usableAsStorageImage = true;
            hbaoResultSpec.textureWrap = TextureWrap::Clamp;
            hbaoResultSpec.mipMapFiltering = MipMapFiltering::Nearest;

            hbaoResult = GraphicsObjectsFactory::createAttachment(hbaoResultSpec);

            DescriptorSetSpecification hbaoComputeDescriptorSetSpec{};
            hbaoComputeDescriptorSetSpec.layout = hbaoComputeDescriptorSetLayout;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings.resize(2);
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[0].attachment = hbaoDeinterleavingAttachment;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[1].attachment = gBufferViewNormalsAttachment;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[1].bindingPoint = 1;
            hbaoComputeDescriptorSetSpec.attachmentTextureBindings[1].allLayers = true;
            hbaoComputeDescriptorSetSpec.attachmentImageBindings.resize(1);
            hbaoComputeDescriptorSetSpec.attachmentImageBindings[0].attachment = hbaoResult;
            hbaoComputeDescriptorSetSpec.attachmentImageBindings[0].bindingPoint = 2;
            hbaoComputeDescriptorSetSpec.attachmentImageBindings[0].allLayers = true;
            hbaoComputeDescriptorSetSpec.attachmentImageBindings[0].access = ShaderImageAccess::WriteOnly;
            hbaoComputeDescriptorSetSpec.uboBindings = {
                    {3, ubScreenData},
                    {4, ubHBAOData}
            };

            hbaoComputeDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(hbaoComputeDescriptorSetSpec);

            AttachmentSpecification hbaoReinterleavingAttachmentSpec{};
            hbaoReinterleavingAttachmentSpec.layerCount = 1;
            hbaoReinterleavingAttachmentSpec.width = viewportWidth;
            hbaoReinterleavingAttachmentSpec.height = viewportHeight;
            hbaoReinterleavingAttachmentSpec.type = AttachmentType::RG16F;
            hbaoReinterleavingAttachmentSpec.usableAsTexture = true;
            hbaoReinterleavingAttachmentSpec.textureWrap = TextureWrap::Clamp;
            hbaoReinterleavingAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;

            hbaoReinterleavingAttachment = GraphicsObjectsFactory::createAttachment(hbaoReinterleavingAttachmentSpec);

            RenderPassSpecification hbaoReinterleavingRenderPassSpec{};
            hbaoReinterleavingRenderPassSpec.clearColorAttachments = true;
            hbaoReinterleavingRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            hbaoReinterleavingRenderPassSpec.clearDepthStencilAttachment = false;

            hbaoReinterleavingRenderPass = GraphicsObjectsFactory::createRenderPass(hbaoReinterleavingRenderPassSpec);

            DescriptorSetLayoutSpecification hbaoReinterleavingDescriptorSetLayoutSpec{};
            hbaoReinterleavingDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Fragment}};
            hbaoReinterleavingDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(hbaoReinterleavingDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification hbaoReinterleavingPipelineSpec{};
            hbaoReinterleavingPipelineSpec.engineShaderName = "hbaoReinterleaving";
            hbaoReinterleavingPipelineSpec.frontfaceCulling = false;
            hbaoReinterleavingPipelineSpec.backfaceCulling = false;
            hbaoReinterleavingPipelineSpec.depthWrite = false;
            hbaoReinterleavingPipelineSpec.depthTest = false;
            hbaoReinterleavingPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            hbaoReinterleavingPipelineSpec.descriptorSetLayouts = {hbaoReinterleavingDescriptorSetLayout};
            hbaoReinterleavingPipelineSpec.colorAttachments = { AttachmentType::RG16F };
            hbaoDeinterleavingPipelineSpec.hasDepthStencilAttachment = false;

            hbaoReinterleavingPipeline = GraphicsObjectsFactory::createGraphicsPipeline(hbaoReinterleavingPipelineSpec);

            FramebufferSpecification hbaoReinterleavingFramebufferSpec{};
            hbaoReinterleavingFramebufferSpec.width = viewportWidth;
            hbaoReinterleavingFramebufferSpec.height = viewportHeight;
            hbaoReinterleavingFramebufferSpec.colorAttachments = {
                    {hbaoReinterleavingAttachment}
            };

            hbaoReinterleavingFramebuffer = GraphicsObjectsFactory::createFramebuffer(hbaoReinterleavingFramebufferSpec);

            DescriptorSetSpecification hbaoReinterleavingDescriptorSetSpec{};
            hbaoReinterleavingDescriptorSetSpec.layout = hbaoReinterleavingDescriptorSetLayout;
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings.resize(1);
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].attachment = hbaoResult;
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;

            hbaoReinterleavingDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(hbaoReinterleavingDescriptorSetSpec);

            AttachmentSpecification hbaoBlurAttachmentSpec{};
            hbaoBlurAttachmentSpec.width = viewportWidth;
            hbaoBlurAttachmentSpec.height = viewportHeight;
            hbaoBlurAttachmentSpec.type = AttachmentType::RG16F;
            hbaoBlurAttachmentSpec.usableAsTexture = true;

            hbaoBlurAttachment0 = GraphicsObjectsFactory::createAttachment(hbaoBlurAttachmentSpec);
            hbaoBlurAttachment1 = GraphicsObjectsFactory::createAttachment(hbaoBlurAttachmentSpec);

            RenderPassSpecification hbaoBlurRenderPassSpec0{};
            hbaoBlurRenderPassSpec0.clearColorAttachments = true;
            hbaoBlurRenderPassSpec0.clearDepthStencilAttachment = false;
            hbaoBlurRenderPassSpec0.clearColor = {1.0f, 1.0f, 1.0f, 0.0f};
            hbaoBlurRenderPass0 = GraphicsObjectsFactory::createRenderPass(hbaoBlurRenderPassSpec0);

            RenderPassSpecification hbaoBlurRenderPassSpec1{};
            hbaoBlurRenderPassSpec1.clearColorAttachments = false;
            hbaoBlurRenderPassSpec1.clearDepthStencilAttachment = false;
            hbaoBlurRenderPass1 = GraphicsObjectsFactory::createRenderPass(hbaoBlurRenderPassSpec1);

            DescriptorSetLayoutSpecification hbaoBlurDescriptorSetLayoutSpec{};
            hbaoBlurDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Fragment}};
            hbaoBlurDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(hbaoBlurDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification hbaoBlurPipelineSpec{};
            hbaoBlurPipelineSpec.engineShaderName = "hbaoBlur";
            hbaoBlurPipelineSpec.frontfaceCulling = false;
            hbaoBlurPipelineSpec.backfaceCulling = false;
            hbaoBlurPipelineSpec.depthTest = false;
            hbaoBlurPipelineSpec.depthWrite = false;
            hbaoBlurPipelineSpec.descriptorSetLayouts = {hbaoBlurDescriptorSetLayout};
            hbaoBlurPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            hbaoBlurPipelineSpec.colorAttachments = { AttachmentType::RG16F };
            hbaoBlurPipelineSpec.hasDepthStencilAttachment = false;
            hbaoBlurPipelineSpec.usesPushConstants = true;
            hbaoBlurPipelineSpec.pushConstantsSize = sizeof(HbaoBlurPushConstants);


            hbaoBlurPipeline = GraphicsObjectsFactory::createGraphicsPipeline(hbaoBlurPipelineSpec);

            FramebufferSpecification hbaoBlurFramebufferSpec0{};
            hbaoBlurFramebufferSpec0.width = viewportWidth;
            hbaoBlurFramebufferSpec0.height = viewportHeight;
            hbaoBlurFramebufferSpec0.colorAttachments = {
                    {hbaoBlurAttachment0}
            };

            hbaoBlurFramebuffer0 = GraphicsObjectsFactory::createFramebuffer(hbaoBlurFramebufferSpec0);

            FramebufferSpecification hbaoBlurFramebufferSpec1{};
            hbaoBlurFramebufferSpec1.width = viewportWidth;
            hbaoBlurFramebufferSpec1.height = viewportHeight;
            hbaoBlurFramebufferSpec1.colorAttachments = {
                    {hbaoBlurAttachment1}
            };

            hbaoBlurFramebuffer1 = GraphicsObjectsFactory::createFramebuffer(hbaoBlurFramebufferSpec1);

            DescriptorSetSpecification hbaoBlurDescriptorSetSpec0{};
            hbaoBlurDescriptorSetSpec0.layout = hbaoBlurDescriptorSetLayout;
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings.resize(1);
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].attachment = hbaoReinterleavingAttachment;
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].allLayers = true;

            hbaoBlurDescriptorSet0 = GraphicsObjectsFactory::createDescriptorSet(hbaoBlurDescriptorSetSpec0);

            DescriptorSetSpecification hbaoBlurDescriptorSetSpec1{};
            hbaoBlurDescriptorSetSpec1.layout = hbaoBlurDescriptorSetLayout;
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings.resize(1);
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].attachment = hbaoBlurAttachment0;
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].allLayers = true;

            hbaoBlurDescriptorSet1 = GraphicsObjectsFactory::createDescriptorSet(hbaoBlurDescriptorSetSpec1);
        }
        {
            RenderPassSpecification pbrRenderPassSpec{};
            pbrRenderPassSpec.clearColorAttachments = true;
            pbrRenderPassSpec.clearDepthStencilAttachment = false;
            pbrRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

            pbrRenderPass = GraphicsObjectsFactory::createRenderPass(pbrRenderPassSpec);

            DescriptorSetLayoutSpecification pbrDescriptorSetLayoutSpec{};
            pbrDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {
                {7, DescriptorSetLayoutBindingUsage::Fragment},
                {8, DescriptorSetLayoutBindingUsage::Fragment},
                {9, DescriptorSetLayoutBindingUsage::Fragment},
                {10, DescriptorSetLayoutBindingUsage::Fragment},
                {11, DescriptorSetLayoutBindingUsage::Fragment},
                {12, DescriptorSetLayoutBindingUsage::Fragment},
                {13, DescriptorSetLayoutBindingUsage::Fragment}
            };
            pbrDescriptorSetLayoutSpec.uboBindingPoints = {
                {0, DescriptorSetLayoutBindingUsage::Fragment},
                {1, DescriptorSetLayoutBindingUsage::Fragment},
                {2, DescriptorSetLayoutBindingUsage::Fragment}
            };
            pbrDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(pbrDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification pbrPipelineSpec{};
            pbrPipelineSpec.engineShaderName = "pbr";
            pbrPipelineSpec.depthWrite = false;
            pbrPipelineSpec.depthTest = false;
            pbrPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            pbrPipelineSpec.descriptorSetLayouts = {pbrDescriptorSetLayout, environmentMapDescriptorSetLayout};
            pbrPipelineSpec.colorAttachments = { AttachmentType::RGBA16F };
            pbrPipelineSpec.hasDepthStencilAttachment = false;
            pbrPipelineSpec.usesPushConstants = true;
            pbrPipelineSpec.pushConstantsSize = sizeof(float);

            pbrPipeline = GraphicsObjectsFactory::createGraphicsPipeline(pbrPipelineSpec);

            AttachmentSpecification pbrColorAttachmentSpec{};
            pbrColorAttachmentSpec.width = viewportWidth;
            pbrColorAttachmentSpec.height = viewportHeight;
            pbrColorAttachmentSpec.type = AttachmentType::RGBA16F;
            pbrColorAttachmentSpec.usableAsTexture = true;
            pbrColorAttachmentSpec.textureWrap = TextureWrap::Clamp;
            pbrColorAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
            pbrColorAttachmentSpec.layerCount = 1;

            pbrColorAttachment = GraphicsObjectsFactory::createAttachment(pbrColorAttachmentSpec);

            FramebufferSpecification pbrFramebufferSpec{};
            pbrFramebufferSpec.height = viewportHeight;
            pbrFramebufferSpec.width = viewportWidth;
            pbrFramebufferSpec.colorAttachments = {
                {pbrColorAttachment}
            };

            pbrFramebuffer = GraphicsObjectsFactory::createFramebuffer(pbrFramebufferSpec);

            DescriptorSetSpecification pbrDescriptorSetSpec{};
            pbrDescriptorSetSpec.layout = pbrDescriptorSetLayout;
            pbrDescriptorSetSpec.attachmentTextureBindings.resize(6);
            pbrDescriptorSetSpec.attachmentTextureBindings[0].attachment = gBufferAlbedoRoughnessAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 10;
            pbrDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].attachment = gBufferEmissionMetallicAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].bindingPoint = 11;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].attachment = gBufferWorldNormalsAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].bindingPoint = 12;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[3].attachment = gBufferDepthAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[3].bindingPoint = 13;
            pbrDescriptorSetSpec.attachmentTextureBindings[3].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[4].attachment = dirShadowMaps;
            pbrDescriptorSetSpec.attachmentTextureBindings[4].bindingPoint = 8;
            pbrDescriptorSetSpec.attachmentTextureBindings[4].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[5].attachment = hbaoBlurAttachment1;
            pbrDescriptorSetSpec.attachmentTextureBindings[5].bindingPoint = 9;
            pbrDescriptorSetSpec.attachmentTextureBindings[5].allLayers = true;
            pbrDescriptorSetSpec.texture2DBindings = {
                {7, Renderer::getBrdfLUTTexture()}
            };
            pbrDescriptorSetSpec.uboBindings = {
                {0, ubCameraData},
                {1, ubLightData},
                {2, ubDirShadowData}
            };

            pbrDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(pbrDescriptorSetSpec);
        }
        {
            RenderPassSpecification afterPbrRenderPassSpec{};
            afterPbrRenderPassSpec.clearColorAttachments = false;
            afterPbrRenderPassSpec.clearDepthStencilAttachment = false;

            afterPbrRenderPass = GraphicsObjectsFactory::createRenderPass(afterPbrRenderPassSpec);

            FramebufferSpecification afterPbrFramebufferSpec{};
            afterPbrFramebufferSpec.width = viewportWidth;
            afterPbrFramebufferSpec.height = viewportHeight;
            afterPbrFramebufferSpec.colorAttachments = {
                    {pbrColorAttachment}
            };
            afterPbrFramebufferSpec.depthStencilAttachment.attachment = gBufferDepthAttachment;
            afterPbrFramebufferSpec.depthStencilAttachment.allLayers = true;

            afterPbrFramebuffer = GraphicsObjectsFactory::createFramebuffer(afterPbrFramebufferSpec);
        }
        {
            DescriptorSetLayoutSpecification skyboxDescriptorSetLayoutSpec{
                .uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}},
            };
            skyboxDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(skyboxDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification skyboxPipelineSpec{};
            skyboxPipelineSpec.descriptorSetLayouts = {skyboxDescriptorSetLayout, environmentMapDescriptorSetLayout};
            skyboxPipelineSpec.engineShaderName = "skybox";
            skyboxPipelineSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            skyboxPipelineSpec.depthTest = true;
            skyboxPipelineSpec.vertexInputLayout = Renderer::getUnitCubeVertexInputLayout();
            skyboxPipelineSpec.hasDepthStencilAttachment = true;
            skyboxPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            skyboxPipelineSpec.colorAttachments = {
                pbrColorAttachment->getType()
            };
            skyboxPipelineSpec.usesPushConstants = true;
            skyboxPipelineSpec.pushConstantsSize = sizeof(SkyboxPushConstants);

            skyboxPipeline = GraphicsObjectsFactory::createGraphicsPipeline(skyboxPipelineSpec);

            DescriptorSetSpecification skyboxDescriptorSetSpec{
                .layout = skyboxDescriptorSetLayout,
                .uboBindings = {
                    {0, ubCameraData}}
            };
            skyboxDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(skyboxDescriptorSetSpec);
        }
        {
            DescriptorSetLayoutSpecification physicsCollidersDescriptorSetLayoutSpec{};
            physicsCollidersDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}};
            physicsCollidersDescriptorSetLayoutSpec.ssboBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Vertex}};
            physicsCollidersDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(physicsCollidersDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification physicsCollidersPipelineSpec;
            physicsCollidersPipelineSpec.engineShaderName = "colliders";
            physicsCollidersPipelineSpec.depthTest = false;
            physicsCollidersPipelineSpec.depthWrite = false;
            physicsCollidersPipelineSpec.wireframe = true;
            physicsCollidersPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;
            physicsCollidersPipelineSpec.descriptorSetLayouts = {physicsCollidersDescriptorSetLayout};
            physicsCollidersPipelineSpec.hasDepthStencilAttachment = true;
            physicsCollidersPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            physicsCollidersPipelineSpec.colorAttachments = {
                pbrColorAttachment->getType()
            };
            physicsCollidersPipelineSpec.usesPushConstants = true;
            physicsCollidersPipelineSpec.pushConstantsSize = sizeof(CollidersPushConstants);

            physicsCollidersPipeline = GraphicsObjectsFactory::createGraphicsPipeline(physicsCollidersPipelineSpec);

            physicsCollidersTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification physicsCollidersDescriptorSetSpec{};
            physicsCollidersDescriptorSetSpec.layout = physicsCollidersDescriptorSetLayout;
            physicsCollidersDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData}
            };
            physicsCollidersDescriptorSetSpec.ssboBindings = {
                    {1, physicsCollidersTransformsBuffer}
            };

            physicsCollidersDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(physicsCollidersDescriptorSetSpec);
        }
        {
            DescriptorSetLayoutSpecification boundingBoxDescriptorSetLayoutSpec{};
            boundingBoxDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}};
            boundingBoxDescriptorSetLayoutSpec.ssboBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Vertex}};
            boundingBoxDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(boundingBoxDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification boundingBoxPipelineSpec;
            boundingBoxPipelineSpec.engineShaderName = "colliders";
            boundingBoxPipelineSpec.depthTest = true;
            boundingBoxPipelineSpec.depthWrite = false;
            boundingBoxPipelineSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            boundingBoxPipelineSpec.wireframe = true;
            boundingBoxPipelineSpec.backfaceCulling = false;
            boundingBoxPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;
            boundingBoxPipelineSpec.descriptorSetLayouts = {boundingBoxDescriptorSetLayout};
            boundingBoxPipelineSpec.hasDepthStencilAttachment = true;
            boundingBoxPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            boundingBoxPipelineSpec.colorAttachments = {
                pbrColorAttachment->getType()
            };
            boundingBoxPipelineSpec.usesPushConstants = true;
            boundingBoxPipelineSpec.pushConstantsSize = sizeof(CollidersPushConstants);

            boundingBoxPipeline = GraphicsObjectsFactory::createGraphicsPipeline(boundingBoxPipelineSpec);

            boundingBoxTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification boundingBoxDescriptorSetSpec{};
            boundingBoxDescriptorSetSpec.layout = boundingBoxDescriptorSetLayout;
            boundingBoxDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData}
            };
            boundingBoxDescriptorSetSpec.ssboBindings = {
                    {1, boundingBoxTransformsBuffer}
            };

            boundingBoxDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(boundingBoxDescriptorSetSpec);
        }
        {
            GraphicsPipelineSpecification mormalsDebugPipelineSpec{};
            mormalsDebugPipelineSpec.engineShaderName = "normalsVisualize";
            mormalsDebugPipelineSpec.depthTest = true;
            mormalsDebugPipelineSpec.depthWrite = false;
            mormalsDebugPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;
            mormalsDebugPipelineSpec.descriptorSetLayouts = {gBufferDescriptorSetLayout};
            mormalsDebugPipelineSpec.hasDepthStencilAttachment = true;
            mormalsDebugPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            mormalsDebugPipelineSpec.colorAttachments = {
                pbrColorAttachment->getType()
            };
            mormalsDebugPipelineSpec.usesPushConstants = true;
            mormalsDebugPipelineSpec.pushConstantsSize = sizeof(CollidersPushConstants);

            normalsDebugPipeline = GraphicsObjectsFactory::createGraphicsPipeline(mormalsDebugPipelineSpec);
        }
        {
            debugLinesVAO = GraphicsObjectsFactory::createVertexArrayObject();
            auto* linesVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(MAX_DEBUG_LINES * 2 * sizeof(float) * 6, VertexBufferUsage::CPUDynamic);
            linesVertexBuffer->setLayout({{ShaderDataType::Float3, false}, {ShaderDataType::Float3, false}});
            debugLinesVAO->addVertexBuffer(linesVertexBuffer);

            DescriptorSetLayoutSpecification debugLinesDescriptorSetLayoutSpec{};
            debugLinesDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}};
            debugLinesDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(debugLinesDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification debugLinesPipelineSpec{};
            debugLinesPipelineSpec.engineShaderName = "lines";
            debugLinesPipelineSpec.depthTest = true;
            debugLinesPipelineSpec.depthWrite = false;
            debugLinesPipelineSpec.drawMode = DrawMode::Lines;
            debugLinesPipelineSpec.descriptorSetLayouts = {debugLinesDescriptorSetLayout};
            debugLinesPipelineSpec.vertexInputLayout = debugLinesVAO->getLayout();
            debugLinesPipelineSpec.hasDepthStencilAttachment = true;
            debugLinesPipelineSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();
            debugLinesPipelineSpec.colorAttachments = {
                pbrColorAttachment->getType()
            };

            debugLinesPipeline = GraphicsObjectsFactory::createGraphicsPipeline(debugLinesPipelineSpec);

            DescriptorSetSpecification debugLinesDescriptorSetSpec{};
            debugLinesDescriptorSetSpec.layout = debugLinesDescriptorSetLayout;
            debugLinesDescriptorSetSpec.uboBindings = {
                {0, ubCameraData}
            };

            debugLinesDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(debugLinesDescriptorSetSpec);
        }
        {
            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;
            for (auto& bloomAttachment: bloomAttachments) {
                AttachmentSpecification bloomAttachmentSpec{};
                bloomAttachmentSpec.width = static_cast<uint32_t>(bloomWidth);
                bloomAttachmentSpec.height = static_cast<uint32_t>(bloomHeight);
                bloomAttachmentSpec.type = AttachmentType::RGBA16F;
                bloomAttachmentSpec.usableAsTexture = true;
                bloomAttachmentSpec.textureWrap = TextureWrap::Clamp;
                bloomAttachmentSpec.mipMapFiltering = MipMapFiltering::Bilinear;
                bloomAttachmentSpec.layerCount = 1;
                bloomAttachment = GraphicsObjectsFactory::createAttachment(bloomAttachmentSpec);

                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }

            RenderPassSpecification bloomDownSamplePassSpec;
            bloomDownSamplePassSpec.clearColorAttachments = true;
            bloomDownSamplePassSpec.clearDepthStencilAttachment = false;

            bloomDownSamplePass = GraphicsObjectsFactory::createRenderPass(bloomDownSamplePassSpec);

            DescriptorSetLayoutSpecification bloomDescriptorSetLayoutSpec{};
            bloomDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Fragment}};
            bloomDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Fragment}};
            bloomDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(bloomDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification bloomDownsamplePipelineSpec{};
            bloomDownsamplePipelineSpec.engineShaderName = "bloomDownSample";
            bloomDownsamplePipelineSpec.depthTest = false;
            bloomDownsamplePipelineSpec.depthWrite = false;
            bloomDownsamplePipelineSpec.descriptorSetLayouts = {bloomDescriptorSetLayout};
            bloomDownsamplePipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            bloomDownsamplePipelineSpec.colorAttachments = { AttachmentType::RGBA16F };
            bloomDownsamplePipelineSpec.hasDepthStencilAttachment = false;
            bloomDownsamplePipelineSpec.usesPushConstants = true;
            bloomDownsamplePipelineSpec.pushConstantsSize = sizeof(uint32_t);

            bloomDownsamplePipeline = GraphicsObjectsFactory::createGraphicsPipeline(bloomDownsamplePipelineSpec);

            for (size_t i = 0; i < bloomDownsampleFramebuffers.size(); i++) {
                FramebufferSpecification bloomFramebufferSpec{};
                bloomFramebufferSpec.width = bloomAttachments[i]->getWidth();
                bloomFramebufferSpec.height = bloomAttachments[i]->getHeight();
                bloomFramebufferSpec.colorAttachments = {
                        {bloomAttachments[i]}
                };
                bloomDownsampleFramebuffers[i] = GraphicsObjectsFactory::createFramebuffer(bloomFramebufferSpec);
            }

            RenderPassSpecification bloomUpSamplePassSpec;
            bloomUpSamplePassSpec.clearColorAttachments = false;
            bloomUpSamplePassSpec.clearDepthStencilAttachment = false;

            bloomUpSamplePass = GraphicsObjectsFactory::createRenderPass(bloomUpSamplePassSpec);

            GraphicsPipelineSpecification bloomUpsamplePipelineSpec{};
            bloomUpsamplePipelineSpec.engineShaderName = "bloomUpSample";
            bloomUpsamplePipelineSpec.depthTest = false;
            bloomUpsamplePipelineSpec.depthWrite = false;
            bloomUpsamplePipelineSpec.useBlending = true;
            bloomUpsamplePipelineSpec.blendingEquation = BlendingEquation::Add;
            bloomUpsamplePipelineSpec.srcBlendingFunction = BlendingFunction::One;
            bloomUpsamplePipelineSpec.destBlendingFunction = BlendingFunction::One;
            bloomUpsamplePipelineSpec.descriptorSetLayouts = {bloomDescriptorSetLayout};
            bloomUpsamplePipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            bloomUpsamplePipelineSpec.colorAttachments = { AttachmentType::RGBA16F };
            bloomUpsamplePipelineSpec.hasDepthStencilAttachment = false;

            bloomUpsamplePipeline = GraphicsObjectsFactory::createGraphicsPipeline(bloomUpsamplePipelineSpec);

            for (size_t i = 0; i < bloomUpsampleFramebuffers.size(); i++) {
                FramebufferSpecification bloomFramebufferSpec{};
                bloomFramebufferSpec.width = bloomAttachments[i]->getWidth();
                bloomFramebufferSpec.height = bloomAttachments[i]->getHeight();
                bloomFramebufferSpec.colorAttachments = {
                        {bloomAttachments[i]}
                };
                bloomUpsampleFramebuffers[i] = GraphicsObjectsFactory::createFramebuffer(bloomFramebufferSpec);
            }

            DescriptorSetSpecification bloomDescriptorSetSpec0{};
            bloomDescriptorSetSpec0.layout = bloomDescriptorSetLayout;
            bloomDescriptorSetSpec0.uboBindings = {
                {0, ubCameraData}
            };
            bloomDescriptorSetSpec0.attachmentTextureBindings.resize(1);
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].attachment = pbrColorAttachment;
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].bindingPoint = 1;
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].allLayers = true;

            bloomDescriptorSets[0] = GraphicsObjectsFactory::createDescriptorSet(bloomDescriptorSetSpec0);

            for (size_t i = 1; i < bloomDescriptorSets.size(); i++) {
                DescriptorSetSpecification bloomDescriptorSetSpec{};
                bloomDescriptorSetSpec.layout = bloomDescriptorSetLayout;
                bloomDescriptorSetSpec.uboBindings = {
                    {0, ubScreenData}
                };
                bloomDescriptorSetSpec.attachmentTextureBindings.resize(1);
                bloomDescriptorSetSpec.attachmentTextureBindings[0].attachment = bloomAttachments[i - 1];
                bloomDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 1;
                bloomDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;

                bloomDescriptorSets[i] = GraphicsObjectsFactory::createDescriptorSet(bloomDescriptorSetSpec);
            }
        }
        {
            DescriptorSetLayoutSpecification screenDescriptorSetLayoutSpec{};
            screenDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Fragment}};
            screenDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {
                {1, DescriptorSetLayoutBindingUsage::Fragment},
                {2, DescriptorSetLayoutBindingUsage::Fragment}
            };
            screenDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(screenDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification screenPipelineSpec;
            screenPipelineSpec.engineShaderName = "screen";
            screenPipelineSpec.depthTest = false;
            screenPipelineSpec.depthWrite = false;
            screenPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            screenPipelineSpec.descriptorSetLayouts = {screenDescriptorSetLayout};

            PipelineAttachmentInfo swapChainAttachmentInfo = Renderer::getSwapChainAttachmentInfo();

            screenPipelineSpec.colorAttachments = swapChainAttachmentInfo.colorAttachments;
            screenPipelineSpec.hasDepthStencilAttachment = swapChainAttachmentInfo.hasDepthStencilAttachment;
            screenPipelineSpec.depthAttachmentFormat = swapChainAttachmentInfo.depthAttachmentFormat;

            screenPipeline = GraphicsObjectsFactory::createGraphicsPipeline(screenPipelineSpec);

            DescriptorSetSpecification screenDescriptorSetSpec{};
            screenDescriptorSetSpec.layout = screenDescriptorSetLayout;
            screenDescriptorSetSpec.uboBindings = {
                {0, ubCameraData}
            };
            screenDescriptorSetSpec.attachmentTextureBindings.resize(2);
            screenDescriptorSetSpec.attachmentTextureBindings[0].attachment = pbrColorAttachment;
            screenDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 1;
            screenDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            screenDescriptorSetSpec.attachmentTextureBindings[1].attachment = bloomAttachments[0];
            screenDescriptorSetSpec.attachmentTextureBindings[1].bindingPoint = 2;
            screenDescriptorSetSpec.attachmentTextureBindings[1].allLayers = true;

            screenDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(screenDescriptorSetSpec);
        }
        {
            auto* uiIndices = new uint32_t[MAX_UI_INDICES];

            uint32_t offset = 0;
            for (uint32_t i = 0; i < MAX_UI_INDICES; i += 6) {
                uiIndices[i + 0] = offset + 0;
                uiIndices[i + 1] = offset + 1;
                uiIndices[i + 2] = offset + 2;

                uiIndices[i + 3] = offset + 2;
                uiIndices[i + 4] = offset + 3;
                uiIndices[i + 5] = offset + 0;

                offset += 4;
            }

            uiIndexBuffer = GraphicsObjectsFactory::createIndexBuffer(uiIndices, MAX_UI_INDICES, IndexBufferDataType::UInt32);

            DescriptorSetLayoutSpecification uiDescriptorSetLayoutSpec{};
            uiDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints.resize(1);
            uiDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].bindingPoint = 0;
            uiDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].usage = DescriptorSetLayoutBindingUsage::Fragment;
            uiDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].descriptorCount = 16;

            uiDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(uiDescriptorSetLayoutSpec);

            DescriptorSetLayoutSpecification uiTextDescriptorSetLayoutSpec{};
            uiTextDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints.resize(1);
            uiTextDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].bindingPoint = 0;
            uiTextDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].usage = DescriptorSetLayoutBindingUsage::Fragment;
            uiTextDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints[0].descriptorCount = 4;

            uiTextDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(uiTextDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification uiCirclePipelineSpec;
            uiCirclePipelineSpec.engineShaderName = "uiCircle";
            uiCirclePipelineSpec.depthTest = false;
            uiCirclePipelineSpec.depthWrite = false;
            uiCirclePipelineSpec.useBlending = true;
            uiCirclePipelineSpec.blendingEquation = BlendingEquation::Add;
            uiCirclePipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiCirclePipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiCirclePipelineSpec.vertexInputLayout = UI_CIRCLE_VERTEX_BUFFER_LAYOUTS;
            uiCirclePipelineSpec.hasDepthStencilAttachment = false;
            uiCirclePipelineSpec.colorAttachments = { UI_CANVAS_ATTACHMENT_TYPE };
            uiCirclePipelineSpec.descriptorSetLayouts = {uiDescriptorSetLayout};
            uiCirclePipelineSpec.usesPushConstants = true;
            uiCirclePipelineSpec.pushConstantsSize = sizeof(UiPushConstants);

            uiCirclePipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiCirclePipelineSpec);

            GraphicsPipelineSpecification uiRectPipelineSpec;
            uiRectPipelineSpec.engineShaderName = "uiRect";
            uiRectPipelineSpec.depthTest = false;
            uiRectPipelineSpec.depthWrite = false;
            uiRectPipelineSpec.useBlending = true;
            uiRectPipelineSpec.blendingEquation = BlendingEquation::Add;
            uiRectPipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiRectPipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiRectPipelineSpec.vertexInputLayout = UI_RECT_VERTEX_BUFFER_LAYOUTS;
            uiRectPipelineSpec.hasDepthStencilAttachment = false;
            uiRectPipelineSpec.colorAttachments = { UI_CANVAS_ATTACHMENT_TYPE };
            uiRectPipelineSpec.descriptorSetLayouts = {uiDescriptorSetLayout};
            uiRectPipelineSpec.usesPushConstants = true;
            uiRectPipelineSpec.pushConstantsSize = sizeof(UiPushConstants);

            uiRectPipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiRectPipelineSpec);

            GraphicsPipelineSpecification uiTextPipelineSpec;
            uiTextPipelineSpec.engineShaderName = "uiText";
            uiTextPipelineSpec.depthTest = false;
            uiTextPipelineSpec.depthWrite = false;
            uiTextPipelineSpec.useBlending = true;
            uiTextPipelineSpec.blendingEquation = BlendingEquation::Add;
            uiTextPipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiTextPipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiTextPipelineSpec.vertexInputLayout = UI_TEXT_VERTEX_BUFFER_LAYOUTS;
            uiTextPipelineSpec.hasDepthStencilAttachment = false;
            uiTextPipelineSpec.colorAttachments = { UI_CANVAS_ATTACHMENT_TYPE };
            uiTextPipelineSpec.descriptorSetLayouts = {uiTextDescriptorSetLayout};
            uiTextPipelineSpec.usesPushConstants = true;
            uiTextPipelineSpec.pushConstantsSize = sizeof(UiPushConstants);

            uiTextPipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiTextPipelineSpec);
        }
        {
            DescriptorSetLayoutSpecification ui2DCameraBufferDescriptorSetLayoutSpec{};
            ui2DCameraBufferDescriptorSetLayoutSpec.uboBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Vertex}};
            ui2DDescriptorSetLayoutCameraBuffer = GraphicsObjectsFactory::createDescriptorSetLayout(ui2DCameraBufferDescriptorSetLayoutSpec);

            DescriptorSetSpecification ui2DCameraBufferDescriptorSetSpec{};
            ui2DCameraBufferDescriptorSetSpec.layout = ui2DDescriptorSetLayoutCameraBuffer;
            ui2DCameraBufferDescriptorSetSpec.uboBindings = {
                {0, ubCameraData}
            };
            ui2DDescriptorSetCameraBuffer = GraphicsObjectsFactory::createDescriptorSet(ui2DCameraBufferDescriptorSetSpec);

            DescriptorSetLayoutSpecification uiCanvasSampleDescriptorSetLayoutSpec{};
            uiCanvasSampleDescriptorSetLayoutSpec.texture2DAndAttachmentBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Fragment}};
            uiCanvasSampleDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(uiCanvasSampleDescriptorSetLayoutSpec);

            GraphicsPipelineSpecification ui2DPipelineSpec;
            ui2DPipelineSpec.engineShaderName = "ui2D";
            ui2DPipelineSpec.depthTest = false;
            ui2DPipelineSpec.depthWrite = false;
            ui2DPipelineSpec.useBlending = true;
            ui2DPipelineSpec.blendingEquation = BlendingEquation::Add;
            ui2DPipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            ui2DPipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            ui2DPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();
            ui2DPipelineSpec.descriptorSetLayouts = {ui2DDescriptorSetLayoutCameraBuffer, uiCanvasSampleDescriptorSetLayout};
            ui2DPipelineSpec.usesPushConstants = true;
            ui2DPipelineSpec.pushConstantsSize = sizeof(glm::mat4);

            PipelineAttachmentInfo swapChainAttachmentInfo = Renderer::getSwapChainAttachmentInfo();

            ui2DPipelineSpec.colorAttachments = swapChainAttachmentInfo.colorAttachments;
            ui2DPipelineSpec.hasDepthStencilAttachment = swapChainAttachmentInfo.hasDepthStencilAttachment;
            ui2DPipelineSpec.depthAttachmentFormat = swapChainAttachmentInfo.depthAttachmentFormat;

            ui2DPipeline = GraphicsObjectsFactory::createGraphicsPipeline(ui2DPipelineSpec);

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }
        {
            boneTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_BONES * MAX_ANIMATED_COMPONENTS * sizeof(glm::mat4));

            DescriptorSetLayoutSpecification animatedMeshDescriptorSetLayoutSpec{};
            animatedMeshDescriptorSetLayoutSpec.immutableSsboBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Compute}};
            animatedMeshDescriptorSetLayoutSpec.vertexBufferSsboBindingPoints = {
                {3, DescriptorSetLayoutBindingUsage::Compute},
                {4, DescriptorSetLayoutBindingUsage::Compute}
            };
            animatedMeshDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(animatedMeshDescriptorSetLayoutSpec);

            DescriptorSetLayoutSpecification skinningDescriptorSetLayoutSpec{};
            skinningDescriptorSetLayoutSpec.ssboBindingPoints = {{2, DescriptorSetLayoutBindingUsage::Compute}};
            skinningDescriptorSetLayout = GraphicsObjectsFactory::createDescriptorSetLayout(skinningDescriptorSetLayoutSpec);

            ComputePipelineSpecification skinningPipelineSpec{};
            skinningPipelineSpec.engineShaderName = "skinning";
            skinningPipelineSpec.descriptorSetLayouts = {skinningDescriptorSetLayout, animatedMeshDescriptorSetLayout};
            skinningPipelineSpec.usesPushConstants = true;
            skinningPipelineSpec.pushConstantsSize = sizeof(SkinningPushConstants);

            skinningComputePipeline = GraphicsObjectsFactory::createComputePipeline(skinningPipelineSpec);

            DescriptorSetSpecification skinningDescriptorSetSpec{};
            skinningDescriptorSetSpec.layout = skinningDescriptorSetLayout;
            skinningDescriptorSetSpec.ssboBindings = {
                    {2, boneTransformsBuffer}
            };

            skinningDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(skinningDescriptorSetSpec);
        }
    }

    SceneRenderer::~SceneRenderer() {
        delete environmentMapDescriptorSetBlack;
        delete environmentMapDescriptorSetLayout;

        delete dirShadowMapDescriptorSet;
        delete dirShadowMapDescriptorSetLayout;
        delete dirShadowMapFramebuffer;
        delete dirShadowMaps;
        delete dirShadowMapTransformsBuffer;
        delete dirShadowMapRenderPass;
        delete dirShadowMapPipeline;

        delete gBufferDescriptorSet;
        delete gBufferDescriptorSetLayout;
        delete gBufferRenderPass;
        delete gBufferPipeline;
        delete gBufferAlbedoRoughnessAttachment;
        delete gBufferEmissionMetallicAttachment;
        delete gBufferWorldNormalsAttachment;
        delete gBufferViewNormalsAttachment;
        delete gBufferDepthAttachment;
        delete gBufferFramebuffer;
        delete gBufferTransformsBuffer;

        delete hbaoDeinterleavingDescriptorSet;
        delete hbaoDeinterleavingDescriptorSetLayout;
        delete hbaoDeinterleavingRenderPass;
        delete hbaoDeinterleavingPipeline;
        delete hbaoDeinterleavingAttachment;
        delete hbaoDeinterleavingFramebuffers[0];
        delete hbaoDeinterleavingFramebuffers[1];

        delete hbaoComputeDescriptorSet;
        delete hbaoComputeDescriptorSetLayout;
        delete hbaoComputePipeline;
        delete hbaoResult;

        delete hbaoReinterleavingRenderPass;
        delete hbaoReinterleavingPipeline;
        delete hbaoReinterleavingAttachment;
        delete hbaoReinterleavingFramebuffer;
        delete hbaoReinterleavingDescriptorSet;
        delete hbaoReinterleavingDescriptorSetLayout;

        delete hbaoBlurRenderPass0;
        delete hbaoBlurRenderPass1;
        delete hbaoBlurPipeline;
        delete hbaoBlurAttachment0;
        delete hbaoBlurAttachment1;
        delete hbaoBlurFramebuffer0;
        delete hbaoBlurFramebuffer1;
        delete hbaoBlurDescriptorSet0;
        delete hbaoBlurDescriptorSet1;
        delete hbaoBlurDescriptorSetLayout;

        delete pbrRenderPass;
        delete pbrPipeline;
        delete pbrColorAttachment;
        delete pbrFramebuffer;
        delete pbrDescriptorSet;
        delete pbrDescriptorSetLayout;

        delete afterPbrRenderPass;
        delete afterPbrFramebuffer;

        delete skyboxPipeline;
        delete skyboxDescriptorSet;
        delete skyboxDescriptorSetLayout;

        delete boundingBoxDescriptorSetLayout;
        delete boundingBoxPipeline;
        delete boundingBoxTransformsBuffer;
        delete boundingBoxDescriptorSet;

        delete physicsCollidersDescriptorSetLayout;
        delete physicsCollidersPipeline;
        delete physicsCollidersTransformsBuffer;
        delete physicsCollidersDescriptorSet;

        delete normalsDebugPipeline;

        delete bloomDescriptorSetLayout;
        delete bloomDownSamplePass;
        delete bloomUpSamplePass;
        delete bloomDownsamplePipeline;
        delete bloomUpsamplePipeline;

        for (auto& attachment : bloomAttachments) {
            delete attachment;
        }
        for (auto& framebuffer : bloomDownsampleFramebuffers) {
            delete framebuffer;
        }
        for (auto& framebuffer : bloomUpsampleFramebuffers) {
            delete framebuffer;
        }
        for (auto& descriptorSet : bloomDescriptorSets) {
            delete descriptorSet;
        }

        delete screenDescriptorSetLayout;
        delete screenPipeline;
        delete screenDescriptorSet;

        delete uiCirclePipeline;
        delete uiRectPipeline;
        delete uiTextPipeline;
        delete uiDescriptorSetLayout;
        delete uiTextDescriptorSetLayout;
        delete uiIndexBuffer;

        delete ui2DPipeline;
        delete ui2DDescriptorSetLayoutCameraBuffer;
        delete ui2DDescriptorSetCameraBuffer;
        delete uiCanvasSampleDescriptorSetLayout;

        delete debugLinesDescriptorSetLayout;
        delete debugLinesPipeline;
        delete debugLinesDescriptorSet;
        delete debugLinesVAO;

        delete customPipelineDescriptorSetLayout;
        delete customPipelineDescriptorSet;

        delete animatedMeshDescriptorSetLayout;
        delete skinningDescriptorSetLayout;
        delete boneTransformsBuffer;
        delete skinningComputePipeline;
        delete skinningDescriptorSet;

        delete ubCameraData;
        delete ubLightData;
        delete ubDirShadowData;
        delete ubScreenData;
        delete ubHBAOData;
    }

    void SceneRenderer::setActiveScene(Scene* scene) {
        activeScene = scene;
    }

    void SceneRenderer::setViewportSize(uint32_t width, uint32_t height) {
        if (width != viewportWidth || height != viewportHeight) {
            needsResize = true;

            viewportWidth = width;
            viewportHeight = height;
            invViewportWidth = 1.0f / static_cast<float>(viewportWidth);
            invViewportHeight = 1.0f / static_cast<float>(viewportHeight);
        }
    }

    void SceneRenderer::beginScene(const Camera& camera, glm::mat4 cameraTransform, const SceneLightEnvironment& lightEnvironment, const SceneEnvironment& sceneEnvironment) {
        CG_ASSERT(!activeRendering, "Already Rendering Scene!")
        CG_ASSERT(activeScene, "No active Scene!")

        resetRenderingStats();

        ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

        activeRendering = true;

        if (needsResize && viewportWidth != 0 && viewportHeight != 0) {
            needsResize = false;

            UBScreenData screenData{};
            screenData.fullResolution = { viewportWidth, viewportHeight };
            screenData.invFullResolution = { invViewportWidth, invViewportHeight};
            screenData.halfResolution = glm::ivec2{ viewportWidth,  viewportHeight } / 2;
            screenData.invHalfResolution = { invViewportWidth * 2.0f,  invViewportHeight * 2.0f };
            ubScreenData->setData(&screenData, sizeof(UBScreenData));

            gBufferAlbedoRoughnessAttachment->resize(viewportWidth, viewportHeight);
            gBufferEmissionMetallicAttachment->resize(viewportWidth, viewportHeight);
            gBufferWorldNormalsAttachment->resize(viewportWidth, viewportHeight);
            gBufferViewNormalsAttachment->resize(viewportWidth, viewportHeight);
            gBufferDepthAttachment->resize(viewportWidth, viewportHeight);
            gBufferFramebuffer->recreate(viewportWidth, viewportHeight);

            glm::uvec2 quarterSize = (glm::uvec2(viewportWidth, viewportHeight) + 3u) / 4u;

            hbaoDeinterleavingAttachment->resize(quarterSize.x, quarterSize.y);
            hbaoDeinterleavingFramebuffers[0]->recreate(quarterSize.x, quarterSize.y);
            hbaoDeinterleavingFramebuffers[1]->recreate(quarterSize.x, quarterSize.y);

            hbaoResult->resize(quarterSize.x, quarterSize.y);

            hbaoReinterleavingAttachment->resize(viewportWidth, viewportHeight);
            hbaoReinterleavingFramebuffer->recreate(viewportWidth, viewportHeight);

            hbaoBlurAttachment0->resize(viewportWidth, viewportHeight);
            hbaoBlurAttachment1->resize(viewportWidth, viewportHeight);
            hbaoBlurFramebuffer0->recreate(viewportWidth, viewportHeight);
            hbaoBlurFramebuffer1->recreate(viewportWidth, viewportHeight);

            constexpr uint32_t HBAO_WORK_GROUP_SIZE = 16u;
            glm::uvec2 quarterSizeWorkGroups = quarterSize + (HBAO_WORK_GROUP_SIZE - quarterSize % HBAO_WORK_GROUP_SIZE);
            hbaoWorkGroupSize.x = quarterSizeWorkGroups.x / 16u;
            hbaoWorkGroupSize.y = quarterSizeWorkGroups.y / 16u;
            hbaoWorkGroupSize.z = 16u;

            pbrColorAttachment->resize(viewportWidth, viewportHeight);
            pbrFramebuffer->recreate(viewportWidth, viewportHeight);

            afterPbrFramebuffer->recreate(viewportWidth, viewportHeight);

            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;

            for (auto& bloomAttachment: bloomAttachments) {
                bloomAttachment->resize(static_cast<uint32_t>(bloomWidth), static_cast<uint32_t>(bloomHeight));
                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }
            for (size_t i = 0; i < bloomDownsampleFramebuffers.size(); i++) {
                bloomDownsampleFramebuffers[i]->recreate(bloomAttachments[i]->getWidth(), bloomAttachments[i]->getHeight());
            }
            for (size_t i = 0; i < bloomUpsampleFramebuffers.size(); i++) {
                bloomUpsampleFramebuffers[i]->recreate(bloomAttachments[i]->getWidth(), bloomAttachments[i]->getHeight());
            }
            for (const auto& item: bloomDescriptorSets) {
                item->recreate();
            }

            hbaoDeinterleavingDescriptorSet->recreate();
            hbaoComputeDescriptorSet->recreate();
            hbaoReinterleavingDescriptorSet->recreate();
            hbaoReinterleavingDescriptorSet->recreate();
            hbaoBlurDescriptorSet0->recreate();
            hbaoBlurDescriptorSet1->recreate();
            pbrDescriptorSet->recreate();
            screenDescriptorSet->recreate();

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

        UBCameraData cameraData{};
        cameraData.projection = camera.getProjectionMatrix();
        cameraData.view = glm::inverse(cameraTransform);
        cameraData.viewProjection = cameraData.projection * cameraData.view;
        cameraData.invViewProjection = glm::inverse(cameraData.viewProjection);
        cameraData.uiProjectionMatrix = uiProjectionMatrix;
        cameraData.position = cameraTransform[3];
        cameraData.clipInfo = {
                camera.getProjectionType() == CameraProjectionType::Perspective ? camera.getPerspectiveFar() * camera.getPerspectiveNear() : camera.getOrthographicFar() * camera.getOrthographicNear(),
                camera.getProjectionType() == CameraProjectionType::Perspective ? camera.getPerspectiveNear() - camera.getPerspectiveFar() : camera.getOrthographicNear() - camera.getOrthographicFar(),
                camera.getProjectionType() == CameraProjectionType::Perspective ? camera.getPerspectiveFar() : camera.getOrthographicFar(),
                camera.getProjectionType() == CameraProjectionType::Perspective ? 1.0f : 0.0f
        };
        cameraData.exposure = camera.getExposure();
        cameraData.bloomIntensity = applicationOptions.enableBloom ? camera.getBloomIntensity() : 0.0f;
        cameraData.bloomThreshold = camera.getBloomThreshold();
        ubCameraData->setData(&cameraData, sizeof(UBCameraData));

        cameraFrustum.updateCameraFrustum(camera, cameraTransform[3], -cameraTransform[2]);
        cameraPosition = cameraTransform[3];

        UBLightData lightData{};
        lightData.dirLightDirection = glm::vec4(lightEnvironment.dirLightDirection, 0.0f);
        lightData.dirLightIntensity = lightEnvironment.dirLightIntensity;
        lightData.dirLightColor = glm::vec4(lightEnvironment.dirLightColor, 0.0f);
        lightData.pointLightCount = lightEnvironment.pointLights.size();
        lightData.spotLightCount = lightEnvironment.spotLights.size();

        size_t indexPL = 0;
        for (const auto& pointLight: lightEnvironment.pointLights) {
            lightData.pointLights[indexPL].color = glm::vec4(pointLight.color, 0.0f);
            lightData.pointLights[indexPL].falloff = pointLight.falloff;
            lightData.pointLights[indexPL].intensity = pointLight.intensity;
            lightData.pointLights[indexPL].radius = pointLight.radius;
            lightData.pointLights[indexPL].position = glm::vec4(pointLight.position, 0.0f);
            indexPL++;
        }

        size_t indexSL = 0;
        for (const auto& spotLight: lightEnvironment.spotLights) {
            lightData.spotLights[indexSL].color = glm::vec4(spotLight.color, 0.0f);
            lightData.spotLights[indexSL].falloff = spotLight.falloff;
            lightData.spotLights[indexSL].intensity = spotLight.intensity;
            lightData.spotLights[indexSL].radius = spotLight.radius;
            lightData.spotLights[indexSL].position = glm::vec4(spotLight.position, 0.0f);
            lightData.spotLights[indexSL].position = glm::vec4(spotLight.position, 0.0f);
            lightData.spotLights[indexSL].direction = glm::vec4(spotLight.direction, 0.0f);
            lightData.spotLights[indexSL].innerAngle = spotLight.innerAngle;
            lightData.spotLights[indexSL].outerAngle = spotLight.outerAngle;
            indexSL++;
        }

        ubLightData->setData(&lightData, sizeof(UBLightData));


        skyboxPushConstants.intensity = sceneEnvironment.environmentIntensity;
        skyboxPushConstants.lod = sceneEnvironment.environmentLod;

        currentSceneEnvironment.environmentIntensity = sceneEnvironment.environmentIntensity;
        currentSceneEnvironment.environmentMapDescriptorSet = sceneEnvironment.environmentMapDescriptorSet != nullptr ? sceneEnvironment.environmentMapDescriptorSet : environmentMapDescriptorSetBlack;
        currentSceneEnvironment.dirLightCastShadows = lightEnvironment.dirLightCastShadows && lightEnvironment.dirLightIntensity != 0.0f;

        setupShadowMapData(lightEnvironment.dirLightDirection, cameraData.viewProjection, camera);

        if (applicationOptions.enableHBAO) {
            setupHBAOData(cameraData.projection, camera);
            hbaoSharpness = camera.getHbaoSharpness();
        }
    }

    void SceneRenderer::endScene() {
        CG_ASSERT(activeRendering, "Not actively rendering!")

        ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

        buildTransformBuffers();
        fillDebugLinesVertexBuffer();

        skinMeshes();
        shadowMapPass();
        uiCanvasPass();
        gBufferPass();
        customShaderPass();

        if (applicationOptions.enableHBAO) {
            hbaoDeinterleavingPass();
            hbaoComputePass();
            hbaoReinterleavingPass();
            hbaoBlurPass();
        } else {
            Renderer::clearPass(hbaoBlurRenderPass0, hbaoBlurFramebuffer1);
        }

        pbrPass();

        Renderer::beginRenderPass(afterPbrRenderPass, afterPbrFramebuffer);

        skyboxPass();

        #ifdef CG_ENABLE_DEBUG_FEATURES
            if (applicationOptions.debugShowPhysicsColliders) {
                physicsCollidersPass();
            }
            if (applicationOptions.debugShowBoundingBoxes) {
                boundingBoxPass();
            }
            if (applicationOptions.debugShowNormals) {
                normalsDebugPass();
            }
        #endif

        if (applicationOptions.debugRenderLines) {
            debugLinesPass();
        }

        Renderer::endRenderPass();

        if (applicationOptions.enableBloom) {
            bloomPass();
        }

        Renderer::injectBarriersForDescriptorSet(screenDescriptorSet);
        for (const auto& command: ui2DDrawCommandQueue) {
            Renderer::injectBarriersForDescriptorSet(command.sampleCanvasDescriptorSet);
        }

        Renderer::beginSwapChainRenderPass();
        screenPass();
        ui2DPass();
        Renderer::endRenderPass();

        skinningQueue.clear();

        drawCommandQueue.clear();
        meshTransforms.clear();

        customShaderDrawCommandQueue.clear();

        shadowMapDrawCommandQueue.clear();
        shadowMapMeshTransforms.clear();

        uiCanvasDrawCommandQueue.clear();
        ui2DDrawCommandQueue.clear();

        #ifdef CG_ENABLE_DEBUG_FEATURES
            physicsCollidersDrawCommandQueue.clear();
            physicsCollidersMeshTransforms.clear();

            boundingBoxDrawCommandQueue.clear();
            boundingBoxMeshTransforms.clear();
        #endif

        debugLinesDrawInfoQueue.clear();

        activeRendering = false;
        activeScene = nullptr;
    }

    void SceneRenderer::submitMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, PBRMaterial* overrideMaterial, bool castShadows, bool enableCulling, const glm::mat4& transform, const std::vector<float>& lodDistances) {
        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            const auto* meshNode = &mesh->getMeshNodes().at(meshNodeIndex);

            glm::mat4 finalTransform = transform * meshNode->transform;

            if (!meshNode->lodMeshNodes.empty()) {
                meshNode = &mesh->getMeshNodes().at(meshNode->lodMeshNodes[findCorrectLodIndex(lodDistances, finalTransform, meshNode->lodMeshNodes.size())]);
            }

            bool isInCameraFrustum = !enableCulling || cameraFrustum.testAABoundingBoxInFrustum(meshNode->aaBoundingBox, finalTransform);

            for (const auto& submeshIndex: meshNode->submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);

                if (isInCameraFrustum) {
                    const PBRMaterial* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
                    MeshKey mk = {mesh->getVAO(), submeshIndex, material->getUuid().getUuid()};

                    meshTransforms[mk].emplace_back(finalTransform);

                    DrawCommand& drawCommand = drawCommandQueue[mk];
                    drawCommand.vao = mesh->getVAO();
                    drawCommand.material = material;
                    drawCommand.baseIndex = submesh.baseIndex;
                    drawCommand.baseVertex = submesh.baseVertex;
                    drawCommand.indexCount = submesh.indexCount;
                    drawCommand.instanceCount++;
                }

                if (castShadows) {
                    MeshKey mk = {mesh->getVAO(), submeshIndex, 0};

                    shadowMapMeshTransforms[mk].emplace_back(finalTransform);

                    DrawCommand& shadowMapDrawCommand = shadowMapDrawCommandQueue[mk];
                    shadowMapDrawCommand.vao = mesh->getVAO();
                    shadowMapDrawCommand.material = nullptr;
                    shadowMapDrawCommand.baseIndex = submesh.baseIndex;
                    shadowMapDrawCommand.baseVertex = submesh.baseVertex;
                    shadowMapDrawCommand.indexCount = submesh.indexCount;
                    shadowMapDrawCommand.instanceCount++;
                }
            }
        }
    }

    void SceneRenderer::submitAnimatedMesh(MeshVertices* mesh, const std::vector<uint32_t>& meshNodes, PBRMaterial* overrideMaterial, bool castShadows, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, VertexArrayObject* skinnedVAO, const DescriptorSet* descriptorSet) {
        CG_ASSERT(boneTransforms.size() <= MAX_BONES, "Mesh contains to many bones")
        CG_ASSERT(skinningQueue.size() < MAX_ANIMATED_COMPONENTS, "Cannot render that many AnimatedMeshRendererComponents")

        uint32_t boneTransformOffset = skinningQueue.size() * MAX_BONES * sizeof(glm::mat4);
        boneTransformsBuffer->setSubData(boneTransformOffset, boneTransforms.data(), boneTransforms.size() * sizeof(glm::mat4));

        SkinningInfo& skinningInfo = skinningQueue.emplace_back();
        skinningInfo.descriptorSet = descriptorSet;
        skinningInfo.numVertices = mesh->getVertices().size();
        skinningInfo.skinnedVertexBuffer = skinnedVAO->getVertexBuffer(0);

        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            const auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);

            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);
                const PBRMaterial* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
                MeshKey mk = {skinnedVAO, submeshIndex, material->getUuid().getUuid()};

                meshTransforms[mk].emplace_back(transform);

                DrawCommand& drawCommand = drawCommandQueue[mk];
                drawCommand.vao = skinnedVAO;
                drawCommand.material = material;
                drawCommand.baseIndex = submesh.baseIndex;
                drawCommand.baseVertex = submesh.baseVertex;
                drawCommand.indexCount = submesh.indexCount;
                drawCommand.instanceCount++;

                if (castShadows) {
                    shadowMapMeshTransforms[mk].emplace_back(transform);

                    DrawCommand& shadowMapDrawCommand = shadowMapDrawCommandQueue[mk];
                    shadowMapDrawCommand.vao = skinnedVAO;
                    shadowMapDrawCommand.material = material;
                    shadowMapDrawCommand.baseIndex = submesh.baseIndex;
                    shadowMapDrawCommand.baseVertex = submesh.baseVertex;
                    shadowMapDrawCommand.indexCount = submesh.indexCount;
                    shadowMapDrawCommand.instanceCount++;
                }
            }
        }
    }

    void SceneRenderer::submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, bool enableCulling, const AABoundingBox* boundingBox, const glm::mat4& transform, CustomGraphicsPipeline* pipeline, uint32_t instanceCount, const std::vector<float>& lodDistances, const DescriptorSet* descriptorSet) {
        if (enableCulling && boundingBox != nullptr && !cameraFrustum.testAABoundingBoxInFrustum(*boundingBox, transform)) {
            return;
        }

        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            const auto* meshNode = &mesh->getMeshNodes().at(meshNodeIndex);

            glm::mat4 finalTransform = transform * meshNode->transform;

            if (!meshNode->lodMeshNodes.empty()) {
                meshNode = &mesh->getMeshNodes().at(meshNode->lodMeshNodes[findCorrectLodIndex(lodDistances, finalTransform, meshNode->lodMeshNodes.size())]);
            }

            bool isInCameraFrustum = !enableCulling || boundingBox != nullptr || cameraFrustum.testAABoundingBoxInFrustum(meshNode->aaBoundingBox, finalTransform);

            if (isInCameraFrustum) {
                for (const auto& submeshIndex: meshNode->submeshIndices) {
                    const Submesh& submesh = submeshes.at(submeshIndex);

                    CustomShaderDrawCommand& drawCommand = customShaderDrawCommandQueue[pipeline].emplace_back();
                    drawCommand.instanceCount = instanceCount;
                    drawCommand.vao = mesh->getVAO();
                    drawCommand.descriptorSet = descriptorSet;
                    drawCommand.baseIndex = submesh.baseIndex;
                    drawCommand.baseVertex = submesh.baseVertex;
                    drawCommand.indexCount = submesh.indexCount;
                    drawCommand.transform = finalTransform;
                }
            }
        }
    }

    void SceneRenderer::submitPhysicsColliderMesh(MeshVertices* mesh, const glm::mat4& transform) {
        const auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNode: mesh->getMeshNodes()) {
            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);
                MeshKey mk = {mesh->getVAO(), submeshIndex, 0};

                physicsCollidersMeshTransforms[mk].emplace_back(transform * meshNode.transform);

                DrawCommand& drawCommand = physicsCollidersDrawCommandQueue[mk];
                drawCommand.vao = mesh->getVAO();
                drawCommand.material = nullptr;
                drawCommand.baseIndex = submesh.baseIndex;
                drawCommand.baseVertex = submesh.baseVertex;
                drawCommand.indexCount = submesh.indexCount;
                drawCommand.instanceCount++;
            }
        }
    }

    void SceneRenderer::submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, Mesh* mesh, const std::vector<uint32_t>& meshNodes, const glm::mat4& transform) {
        for (const auto& meshNodeIndex: meshNodes) {
            const auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);
            auto [center, extents] = meshNode.aaBoundingBox.getTransformedAdjustedCenterAndExtents(transform * meshNode.transform);
            const auto& boundingBoxSubmesh = boundingBoxMesh->getSubmeshes().at(0);

            MeshKey mk = {boundingBoxMesh->getVAO(), 0, 0};
            boundingBoxMeshTransforms[mk].emplace_back(glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), extents * 2.0f));

            DrawCommand& drawCommand = boundingBoxDrawCommandQueue[mk];
            drawCommand.vao = boundingBoxMesh->getVAO();
            drawCommand.material = nullptr;
            drawCommand.baseIndex = boundingBoxSubmesh.baseIndex;
            drawCommand.baseVertex = boundingBoxSubmesh.baseVertex;
            drawCommand.indexCount = boundingBoxSubmesh.indexCount;
            drawCommand.instanceCount++;
        }
    }

    void SceneRenderer::submitBoundingBoxMesh(MeshVertices* boundingBoxMesh, const AABoundingBox& boundingBox, const glm::mat4& transform) {
        auto [center, extents] = boundingBox.getTransformedAdjustedCenterAndExtents(transform);

        const auto& boundingBoxSubmesh = boundingBoxMesh->getSubmeshes().at(0);

        MeshKey mk = {boundingBoxMesh->getVAO(), 0, 0};
        boundingBoxMeshTransforms[mk].emplace_back(glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), extents * 2.0f));

        DrawCommand& drawCommand = boundingBoxDrawCommandQueue[mk];
        drawCommand.vao = boundingBoxMesh->getVAO();
        drawCommand.material = nullptr;
        drawCommand.baseIndex = boundingBoxSubmesh.baseIndex;
        drawCommand.baseVertex = boundingBoxSubmesh.baseVertex;
        drawCommand.indexCount = boundingBoxSubmesh.indexCount;
        drawCommand.instanceCount++;
    }

    void SceneRenderer::submitDebugLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color) {
        auto& lineInfo = debugLinesDrawInfoQueue.emplace_back();
        lineInfo.from = from;
        lineInfo.to = to;
        lineInfo.color = color;
    }

    void SceneRenderer::submitUiCanvas2D(UiCanvas* uiCanvas, glm::mat4 finalTransform, uint32_t zIndex) {
        auto& command = uiCanvasDrawCommandQueue.emplace_back();
        command.attachment = uiCanvas->getUiAttachment();
        command.projectionMatrix = uiCanvas->getUiProjectionMatrix();
        command.pixelSize = uiCanvas->getPixelSize();
        command.drawCommands = std::move(uiCanvas->getUiDrawCommands());

        auto& command2d = ui2DDrawCommandQueue.emplace_back();
        command2d.sampleCanvasDescriptorSet = uiCanvas->getAttachmentSamplerDescriptorSet();
        command2d.finalTransform = finalTransform;
        command2d.zIndex = zIndex;
    }

    const CameraFrustum& SceneRenderer::getCamaraFrustum() const {
        return cameraFrustum;
    }

    PipelineAttachmentInfo SceneRenderer::getGBufferAttachmentInfo() const {
        PipelineAttachmentInfo info{};

        info.colorAttachments = {
            AttachmentType::RGBA16F,
            AttachmentType::RGBA16F,
            AttachmentType::RGBA16F,
            AttachmentType::RGBA16F
        };
        info.hasDepthStencilAttachment = true;
        info.depthAttachmentFormat = gBufferDepthAttachment->getDepthStencilAttachmentFormat();

        return info;
    }

    const DescriptorSetLayout* SceneRenderer::getCustomPipelineDescriptorSetLayout() const {
        return customPipelineDescriptorSetLayout;
    }

    const IndexBuffer* SceneRenderer::getUiIndexBuffer() const {
        return uiIndexBuffer;
    }

    const DescriptorSetLayout* SceneRenderer::getUiCanvasSampleDescriptorSetLayout() const {
        return uiCanvasSampleDescriptorSetLayout;
    }

    const DescriptorSetLayout * SceneRenderer::getUiDescriptorSetLayout() const {
        return uiDescriptorSetLayout;
    }

    const DescriptorSetLayout * SceneRenderer::getUiTextDescriptorSetLayout() const {
        return uiTextDescriptorSetLayout;
    }

    const DescriptorSetLayout* SceneRenderer::getEnvironmentMapDescriptorSetLayout() const {
        return environmentMapDescriptorSetLayout;
    }

    const DescriptorSetLayout* SceneRenderer::getPBRMaterialDescriptorSetLayout() const {
        return pbrMaterialDescriptorSetLayout;
    }

    const DescriptorSetLayout* SceneRenderer::getAnimatedMeshDescriptorSetLayout() const {
        return animatedMeshDescriptorSetLayout;
    }

    const RenderingStats& SceneRenderer::getRenderingStats() {
        return renderingStats;
    }

    void SceneRenderer::skinMeshes() {
        CG_GPU_DEBUG_GROUP("SkinMeshes")
        CG_GPU_TIME_FN(&renderingStats.skinMeshesTimer)

        SkinningPushConstants pc{};

        Renderer::bindComputePipeline(skinningComputePipeline);
        Renderer::bindDescriptorSet(skinningDescriptorSet, 0);

        for (uint32_t i = 0; i < skinningQueue.size(); i++) {
            Renderer::bindDescriptorSet(skinningQueue[i].descriptorSet, 1);
            pc.componentIndex = i;
            Renderer::setPushConstants(&pc, sizeof(SkinningPushConstants));
            Renderer::dispatchCompute((skinningQueue[i].numVertices / 32) + 1, 1, 1);
            Renderer::memoryBarrierForVertexBufferAfterCompute(skinningQueue[i].skinnedVertexBuffer);
        }
    }

    void SceneRenderer::shadowMapPass() {
        CG_GPU_DEBUG_GROUP("ShadowMapPass")
        CG_GPU_TIME_FN(&renderingStats.shadowMapTimer)

        if (!currentSceneEnvironment.dirLightCastShadows) {
            Renderer::clearPass(dirShadowMapRenderPass, dirShadowMapFramebuffer);
            return;
        }

        Renderer::injectBarriersForDescriptorSet(dirShadowMapDescriptorSet);
        Renderer::beginRenderPass(dirShadowMapRenderPass, dirShadowMapFramebuffer);
        Renderer::bindGraphicsPipeline(dirShadowMapPipeline);
        Renderer::bindDescriptorSet(dirShadowMapDescriptorSet, 0);

        for (const auto [mk, command]: shadowMapDrawCommandQueue) {
            Renderer::setPushConstants(&command.transformsBufferOffset, sizeof(int));
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::gBufferPass() {
        CG_GPU_DEBUG_GROUP("GBufferPass")
        CG_GPU_TIME_FN(&renderingStats.gBufferTimer)

        Renderer::beginRenderPass(gBufferRenderPass, gBufferFramebuffer);
        Renderer::bindGraphicsPipeline(gBufferPipeline);
        Renderer::bindDescriptorSet(gBufferDescriptorSet, 0);

        for (const auto [mk, command]: drawCommandQueue) {
            Renderer::setPushConstants(&command.transformsBufferOffset, sizeof(int));
            Renderer::bindDescriptorSet(command.material->getDescriptorSet(), 1);
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoDeinterleavingPass() {
        CG_GPU_DEBUG_GROUP("HBAODeinterleavingPass")
        CG_GPU_TIME_FN(&renderingStats.hbaoDeinterleavingTimer)

        int uvOffset = 0;

        Renderer::injectBarriersForDescriptorSet(hbaoDeinterleavingDescriptorSet);
        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass, hbaoDeinterleavingFramebuffers[0]);
        Renderer::bindGraphicsPipeline(hbaoDeinterleavingPipeline);
        Renderer::bindDescriptorSet(hbaoDeinterleavingDescriptorSet, 0);
        Renderer::setPushConstants(&uvOffset, sizeof(int));
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        uvOffset = 1;

        Renderer::injectBarriersForDescriptorSet(hbaoDeinterleavingDescriptorSet);
        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass, hbaoDeinterleavingFramebuffers[1]);
        Renderer::bindGraphicsPipeline(hbaoDeinterleavingPipeline);
        Renderer::bindDescriptorSet(hbaoDeinterleavingDescriptorSet, 0);
        Renderer::setPushConstants(&uvOffset, sizeof(int));
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoComputePass() {
        CG_GPU_DEBUG_GROUP("HBAOComputePass")
        CG_GPU_TIME_FN(&renderingStats.hbaoComputeTimer)

        Renderer::injectBarriersForDescriptorSet(hbaoComputeDescriptorSet);
        Renderer::bindComputePipeline(hbaoComputePipeline);
        Renderer::bindDescriptorSet(hbaoComputeDescriptorSet, 0);
        Renderer::dispatchCompute(hbaoWorkGroupSize.x, hbaoWorkGroupSize.y, hbaoWorkGroupSize.z);
        Renderer::transitionImageLayoutFromComputeToShaderReadOnly(hbaoResult, ShaderStage::Fragment);
    }

    void SceneRenderer::hbaoReinterleavingPass() {
        CG_GPU_DEBUG_GROUP("HBAOReinterleavingPass")
        CG_GPU_TIME_FN(&renderingStats.hbaoReinterleavingTimer)

        Renderer::injectBarriersForDescriptorSet(hbaoReinterleavingDescriptorSet);
        Renderer::beginRenderPass(hbaoReinterleavingRenderPass, hbaoReinterleavingFramebuffer);
        Renderer::bindGraphicsPipeline(hbaoReinterleavingPipeline);
        Renderer::bindDescriptorSet(hbaoReinterleavingDescriptorSet, 0);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoBlurPass() {
        CG_GPU_DEBUG_GROUP("HBAOBlurPass")
        CG_GPU_TIME_FN(&renderingStats.hbaoBlurTimer)

        HbaoBlurPushConstants pc{};
        pc.sharpness = hbaoSharpness;
        pc.invResolutionDirection = glm::vec2(invViewportWidth, 0.0f);

        Renderer::injectBarriersForDescriptorSet(hbaoBlurDescriptorSet0);
        Renderer::beginRenderPass(hbaoBlurRenderPass0, hbaoBlurFramebuffer0);
        Renderer::bindGraphicsPipeline(hbaoBlurPipeline);
        Renderer::bindDescriptorSet(hbaoBlurDescriptorSet0, 0);
        Renderer::setPushConstants(&pc, sizeof(HbaoBlurPushConstants));
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        Renderer::injectBarriersForDescriptorSet(hbaoBlurDescriptorSet1);
        Renderer::beginRenderPass(hbaoBlurRenderPass1, hbaoBlurFramebuffer1);
        Renderer::bindGraphicsPipeline(hbaoBlurPipeline);
        Renderer::bindDescriptorSet(hbaoBlurDescriptorSet1, 0);
        pc.invResolutionDirection = glm::vec2(0.0f, invViewportHeight);
        Renderer::setPushConstants(&pc, sizeof(HbaoBlurPushConstants));
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::pbrPass() {
        CG_GPU_DEBUG_GROUP("PBRPass")
        CG_GPU_TIME_FN(&renderingStats.pbrTimer)

        Renderer::injectBarriersForDescriptorSet(pbrDescriptorSet);
        Renderer::injectBarriersForDescriptorSet(currentSceneEnvironment.environmentMapDescriptorSet);
        Renderer::beginRenderPass(pbrRenderPass, pbrFramebuffer);
        Renderer::bindGraphicsPipeline(pbrPipeline);
        Renderer::bindDescriptorSet(pbrDescriptorSet, 0);
        Renderer::bindDescriptorSet(currentSceneEnvironment.environmentMapDescriptorSet, 1);
        Renderer::setPushConstants(&currentSceneEnvironment.environmentIntensity, sizeof(float));
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::customShaderPass() {
        CG_GPU_DEBUG_GROUP("CustomShaderPass")
        CG_GPU_TIME_FN(&renderingStats.customShaderTimer)

        // TODO fix, barriers and clearing

        CustomPipelineData customPipelineData{};

        for (const auto& [pipeline, commands]: customShaderDrawCommandQueue) {
            for (const auto& command: commands) {
                Renderer::injectBarriersForDescriptorSet(command.descriptorSet);
                Renderer::beginRenderPass(gBufferRenderPass, gBufferFramebuffer);

                Renderer::bindGraphicsPipeline(pipeline->getGraphicsPipeline());
                Renderer::bindDescriptorSet(customPipelineDescriptorSet, 0);

                if (command.descriptorSet != nullptr) {
                    Renderer::bindDescriptorSet(command.descriptorSet, 1);
                }

                customPipelineData.transform = command.transform;
                Renderer::setPushConstants(&customPipelineData, sizeof(CustomPipelineData));
                Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);

                Renderer::endRenderPass();
            }
        }
    }

    void SceneRenderer::skyboxPass() {
        CG_GPU_DEBUG_GROUP("SkyboxPass")
        CG_GPU_TIME_FN(&renderingStats.skyboxTimer)

        Renderer::bindGraphicsPipeline(skyboxPipeline);
        Renderer::bindDescriptorSet(skyboxDescriptorSet, 0);
        Renderer::bindDescriptorSet(currentSceneEnvironment.environmentMapDescriptorSet, 1);
        Renderer::setPushConstants(&skyboxPushConstants, sizeof(SkyboxPushConstants));
        Renderer::renderUnitCube();
    }

    void SceneRenderer::physicsCollidersPass() {
        CG_GPU_DEBUG_GROUP("PhysicsCollidersPass")

        Renderer::bindGraphicsPipeline(physicsCollidersPipeline);
        Renderer::bindDescriptorSet(physicsCollidersDescriptorSet, 0);

        CollidersPushConstants pc{};
        pc.color = {0.0f, 1.0f, 0.0f};

        for (const auto [mk, command]: physicsCollidersDrawCommandQueue) {
            pc.transformsOffset = command.transformsBufferOffset;
            Renderer::setPushConstants(&pc, sizeof(CollidersPushConstants));
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }
    }

    void SceneRenderer::boundingBoxPass() {
        CG_GPU_DEBUG_GROUP("BoundingBoxPass")

        Renderer::bindGraphicsPipeline(boundingBoxPipeline);
        Renderer::bindDescriptorSet(boundingBoxDescriptorSet, 0);

        CollidersPushConstants pc{};
        pc.color = {1.0f, 1.0f, 0.0f};

        for (const auto [mk, command]: boundingBoxDrawCommandQueue) {
            pc.transformsOffset = command.transformsBufferOffset;
            Renderer::setPushConstants(&pc, sizeof(CollidersPushConstants));
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }
    }

    void SceneRenderer::normalsDebugPass() {
        CG_GPU_DEBUG_GROUP("NormalsDebugPass")

        Renderer::bindGraphicsPipeline(normalsDebugPipeline);
        Renderer::bindDescriptorSet(gBufferDescriptorSet, 0);

        CollidersPushConstants pc{};
        pc.color = {1.0f, 0.0f, 0.0f};

        for (const auto [mk, command]: drawCommandQueue) {
            pc.transformsOffset = command.transformsBufferOffset;
            Renderer::setPushConstants(&pc, sizeof(CollidersPushConstants));
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }
    }

    void SceneRenderer::debugLinesPass() {
        CG_GPU_DEBUG_GROUP("DebugLinesPass")

        Renderer::bindGraphicsPipeline(debugLinesPipeline);
        Renderer::bindDescriptorSet(debugLinesDescriptorSet, 0);
        Renderer::drawArrays(debugLinesVAO, debugLinesDrawInfoQueue.size() * 2);
    }

    void SceneRenderer::bloomPass() {
        CG_GPU_DEBUG_GROUP("BloomPass")
        CG_GPU_TIME_FN(&renderingStats.bloomTimer)

        uint32_t useThreshold = 1;

        Renderer::injectBarriersForDescriptorSet(bloomDescriptorSets[0]);
        Renderer::beginRenderPass(bloomDownSamplePass, bloomDownsampleFramebuffers[0]);
        Renderer::bindGraphicsPipeline(bloomDownsamplePipeline);
        Renderer::setPushConstants(&useThreshold, sizeof(uint32_t));
        Renderer::bindDescriptorSet(bloomDescriptorSets[0], 0);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        useThreshold = 0;

        for (uint32_t i = 0; i < bloomDownsampleFramebuffers.size() - 1; ++i) {
            Renderer::injectBarriersForDescriptorSet(bloomDescriptorSets[i + 1]);
            Renderer::beginRenderPass(bloomDownSamplePass, bloomDownsampleFramebuffers[i + 1]);
            Renderer::bindGraphicsPipeline(bloomDownsamplePipeline);
            Renderer::setPushConstants(&useThreshold, sizeof(uint32_t));
            Renderer::bindDescriptorSet(bloomDescriptorSets[i + 1], 0);
            Renderer::renderUnitQuad();
            Renderer::endRenderPass();
        }

        for (int32_t i = bloomUpsampleFramebuffers.size() - 1; i >= 0; i--) {
            Renderer::injectBarriersForDescriptorSet(bloomDescriptorSets[i + 2]);
            Renderer::beginRenderPass(bloomUpSamplePass, bloomUpsampleFramebuffers[i]);
            Renderer::bindGraphicsPipeline(bloomUpsamplePipeline);
            Renderer::bindDescriptorSet(bloomDescriptorSets[i + 2], 0);
            Renderer::renderUnitQuad();
            Renderer::endRenderPass();
        }
    }

    void SceneRenderer::screenPass() {
        CG_GPU_DEBUG_GROUP("ScreenPass")
        CG_GPU_TIME_FN(&renderingStats.screenTimer)

        Renderer::bindGraphicsPipeline(screenPipeline);
        Renderer::bindDescriptorSet(screenDescriptorSet, 0);
        Renderer::renderUnitQuad();
    }

    void SceneRenderer::uiCanvasPass() {
        CG_GPU_DEBUG_GROUP("UiCanvasPass")
        CG_GPU_TIME_FN(&renderingStats.uiCanvasTimer)

        UiPushConstants uiPushConstantsData{};

        for (const auto& canvasCommand: uiCanvasDrawCommandQueue) {
            DynamicRenderingInfo renderingInfo{};
            renderingInfo.clearColorAttachments = true;
            renderingInfo.clearDepthStencilAttachment = false;
            renderingInfo.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            renderingInfo.renderArea = canvasCommand.pixelSize;
            renderingInfo.colorAttachments.resize(1);
            renderingInfo.colorAttachments[0].attachment = canvasCommand.attachment;

            for (const auto& command: canvasCommand.drawCommands) {
                Renderer::injectBarriersForDescriptorSet(command.descriptorSet);
                Renderer::injectBarriersForDescriptorSet(command.textDescriptorSet);
            }

            Renderer::beginDynamicRendering(renderingInfo);

            uiPushConstantsData.projection = canvasCommand.projectionMatrix;

            size_t circleOffset = 0;
            size_t rectOffset = 0;
            size_t textOffset = 0;

            for (const auto& command: canvasCommand.drawCommands) {
                if (command.circleIndexCount > 0) {
                    Renderer::bindGraphicsPipeline(uiCirclePipeline);
                    Renderer::setPushConstants(&uiPushConstantsData, sizeof(UiPushConstants));
                    Renderer::bindDescriptorSet(command.descriptorSet, 0);
                    Renderer::executeDrawCommand(command.circleVAO, command.circleIndexCount, 0, circleOffset);

                    circleOffset += command.circleVertexCount;
                }
                if (command.rectIndexCount > 0) {
                    Renderer::bindGraphicsPipeline(uiRectPipeline);
                    Renderer::setPushConstants(&uiPushConstantsData, sizeof(UiPushConstants));
                    Renderer::bindDescriptorSet(command.descriptorSet, 0);
                    Renderer::executeDrawCommand(command.rectVAO, command.rectIndexCount, 0, rectOffset);

                    rectOffset += command.rectVertexCount;
                }
                if (command.textIndexCount > 0) {
                    Renderer::bindGraphicsPipeline(uiTextPipeline);
                    Renderer::setPushConstants(&uiPushConstantsData, sizeof(UiPushConstants));
                    Renderer::bindDescriptorSet(command.textDescriptorSet, 0);
                    Renderer::executeDrawCommand(command.textVAO, command.textIndexCount, 0, textOffset);

                    textOffset += command.textVertexCount;
                }
            }

            Renderer::endDynamicRendering();
        }
    }

    void SceneRenderer::ui2DPass() {
        CG_GPU_DEBUG_GROUP("UiCanvasPass")
        CG_GPU_TIME_FN(&renderingStats.ui2DTimer)

        std::sort(ui2DDrawCommandQueue.begin(), ui2DDrawCommandQueue.end(), [](const auto& a, const auto& b) {
            return a.zIndex < b.zIndex;
        });

        for (const auto& command: ui2DDrawCommandQueue) {
            Renderer::bindGraphicsPipeline(ui2DPipeline);
            Renderer::bindDescriptorSet(ui2DDescriptorSetCameraBuffer, 0);
            Renderer::bindDescriptorSet(command.sampleCanvasDescriptorSet, 1);
            Renderer::setPushConstants(&command.finalTransform, sizeof(glm::mat4));
            Renderer::renderUnitQuad();
        }
    }

    void SceneRenderer::setupShadowMapData(glm::vec3 dirLightDirection, const glm::mat4& cameraViewProjection, const Camera& camera) {
        glm::vec4 cascadeSplits = {0.02f, 0.05f, 0.15f, 0.4f};
        glm::mat4 invCamViewProj = glm::inverse(cameraViewProjection);

        // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering
        // https://learnopengl.com/Guest-Articles/2021/CSM

        UBDirShadowData dirShadowData{};

        float nearToFarPlane = camera.getPerspectiveFar() - camera.getPerspectiveNear();
        float lastSplitDist = 0.0;

        for (int i = 0; i < 4; i++) {
            glm::vec3 frustumCorners[8] = {
                    glm::vec3(-1.0f,  1.0f, -1.0f),
                    glm::vec3( 1.0f,  1.0f, -1.0f),
                    glm::vec3( 1.0f, -1.0f, -1.0f),
                    glm::vec3(-1.0f, -1.0f, -1.0f),
                    glm::vec3(-1.0f,  1.0f,  1.0f),
                    glm::vec3( 1.0f,  1.0f,  1.0f),
                    glm::vec3( 1.0f, -1.0f,  1.0f),
                    glm::vec3(-1.0f, -1.0f,  1.0f)
            };

            for (auto& frustumCorner: frustumCorners) {
                glm::vec4 invFrustumCorner = invCamViewProj * glm::vec4(frustumCorner, 1.0f);
                frustumCorner = invFrustumCorner / invFrustumCorner.w;
            }

            float splitDist = cascadeSplits[i];

            for (uint32_t j = 0; j < 4; j++) {
                glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
                frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
            }
            lastSplitDist = cascadeSplits[i];
            dirShadowData.cascadeSplits[i] = camera.getPerspectiveNear() + splitDist * nearToFarPlane;

            auto frustumCenter = glm::vec3(0.0f);
            for (auto& frustumCorner: frustumCorners) {
                frustumCenter += frustumCorner;
            }
            frustumCenter /= 8.0f;

            const auto lightView = glm::lookAt(frustumCenter + dirLightDirection, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));

            float radius = 0.0f;
            for (auto& frustumCorner: frustumCorners) {
                float distance = glm::length(frustumCorner - frustumCenter);
                radius = glm::max(radius, distance);
            }
            radius = glm::ceil(radius);

            auto maxOrtho = glm::vec3(radius);
            glm::vec3 minOrtho = -maxOrtho;

            glm::mat4 lightProjection = glm::ortho(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, -50.0f, maxOrtho.z - minOrtho.z + 50.0f);

            glm::mat4 shadowMatrix = lightProjection * lightView;
            float shadowMapResolution = static_cast<float>(dirShadowMapFramebuffer->getWidth());
            glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * shadowMapResolution / 2.0f;
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * 2.0f / shadowMapResolution;
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;

            lightProjection[3] += roundOffset;

            dirShadowData.lightSpaceMat[i] = lightProjection * lightView;
        }

        ubDirShadowData->setData(&dirShadowData, sizeof(UBDirShadowData));
    }

    void SceneRenderer::setupHBAOData(const glm::mat4& cameraProjection, const Camera& camera) {
        // From: https://github.com/nvpro-samples/gl_ssao/blob/master/ssao.cpp#L701

        const float* P = glm::value_ptr(cameraProjection);

        const glm::vec4 projInfoPerspective = {
            2.0f / (P[4 * 0 + 0]),                  // (x) * (R - L)/N
            2.0f / (P[4 * 1 + 1]),                  // (y) * (T - B)/N
            -(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0],  // L/N
            -(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1],  // B/N
        };

        const glm::vec4 projInfoOrtho = {
            2.0f / (P[4 * 0 + 0]),                  // ((x) * R - L)
            2.0f / (P[4 * 1 + 1]),                  // ((y) * T - B)
            -(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0],  // L
            -(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1],  // B
        };

        hbaoData.isOrtho = camera.getProjectionType() == CameraProjectionType::Orthographic ? 1 : 0;
        hbaoData.perspectiveInfo = camera.getProjectionType() == CameraProjectionType::Orthographic ? projInfoOrtho : projInfoPerspective;

        const float meters2viewSpace = 1.0f;
        const float R = camera.getHbaoRadius() * meters2viewSpace;
        const float R2 = R * R;
        hbaoData.negInvR2 = -1.0f / R2;
        hbaoData.radiusToScreen = R * 0.5f * (camera.getProjectionType() == CameraProjectionType::Orthographic ? (static_cast<float>(viewportHeight) / projInfoOrtho[1]) : (static_cast<float>(viewportHeight) / (glm::tan(glm::radians(camera.getPerspectiveFov())) * 0.5f) * 2.0f));

        hbaoData.powExponent = glm::max(camera.getHbaoIntensity(), 0.0f);
        hbaoData.nDotVBias = glm::min(std::max(0.f, camera.getHbaoBias()), 1.0f);
        hbaoData.aoMultiplier = 1.0f / (1.0f - hbaoData.nDotVBias);

        hbaoData.invQuarterResolution = 1.0f / glm::vec2{ static_cast<float>(viewportWidth) / 4, static_cast<float>(viewportHeight) / 4 };

        ubHBAOData->setData(&hbaoData, sizeof(UBHBAOData));
    }

    std::array<glm::vec4, 16> SceneRenderer::generateHBAOJitterNoise() const {
        // From: https://github.com/nvpro-samples/gl_ssao/blob/master/ssao.cpp#L325

        std::mt19937 rmt;
        float numDir = 8;  // keep in sync to glsl

        std::array<glm::vec4, 16> result {};

        for (int i = 0; i < 16; i++) {
            float Rand1 = static_cast<float>(rmt()) / 4294967296.0f;
            float Rand2 = static_cast<float>(rmt()) / 4294967296.0f;

            // Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
            const float Angle = glm::two_pi<float>() * Rand1 / numDir;
            result[i].x = glm::cos(Angle);
            result[i].y = glm::sin(Angle);
            result[i].z = Rand2;
            result[i].w = 0;
        }

        return result;
    }

    size_t SceneRenderer::findCorrectLodIndex(const std::vector<float>& lodDistances, const glm::mat4& transform, size_t lodCount) const {
        float distance = glm::length(cameraPosition - glm::vec3(transform[3]));

        size_t out = 0;
        for (size_t i = 0; i < lodDistances.size(); i++) {
            if (distance < lodDistances[i]) {
                break;
            }
            out = i;
        }

        if (out >= lodCount) {
            out = lodCount - 1;
        }

        return out;
    }

    void SceneRenderer::buildTransformBuffers() {
        {
            int currentOffset = 0;
            for (auto& [mk, command]: shadowMapDrawCommandQueue) {
                const auto& transforms = shadowMapMeshTransforms.at(mk);
                uint32_t instanceCount = command.instanceCount;

                command.transformsBufferOffset = currentOffset;
                dirShadowMapTransformsBuffer->setSubData(currentOffset * sizeof(glm::mat4), transforms.data(), instanceCount * sizeof(glm::mat4));

                currentOffset += static_cast<int>(instanceCount);
            }
        }
        {
            int currentOffset = 0;
            for (auto& [mk, command]: drawCommandQueue) {
                const auto& transforms = meshTransforms.at(mk);
                uint32_t instanceCount = command.instanceCount;

                command.transformsBufferOffset = currentOffset;
                gBufferTransformsBuffer->setSubData(currentOffset * sizeof(glm::mat4), transforms.data(), instanceCount * sizeof(glm::mat4));

                currentOffset += static_cast<int>(instanceCount);
            }
        }
        #ifdef CG_ENABLE_DEBUG_FEATURES
            {
                int currentOffset = 0;
                for (auto& [mk, command]: boundingBoxDrawCommandQueue) {
                    const auto& transforms = boundingBoxMeshTransforms.at(mk);
                    uint32_t instanceCount = command.instanceCount;

                    command.transformsBufferOffset = currentOffset;
                    boundingBoxTransformsBuffer->setSubData(currentOffset * sizeof(glm::mat4), transforms.data(), instanceCount * sizeof(glm::mat4));

                    currentOffset += static_cast<int>(instanceCount);
                }
            }
            {
                int currentOffset = 0;
                for (auto& [mk, command]: physicsCollidersDrawCommandQueue) {
                    const auto& transforms = physicsCollidersMeshTransforms.at(mk);
                    uint32_t instanceCount = command.instanceCount;

                    command.transformsBufferOffset = currentOffset;
                    physicsCollidersTransformsBuffer->setSubData(currentOffset * sizeof(glm::mat4), transforms.data(), instanceCount * sizeof(glm::mat4));

                    currentOffset += static_cast<int>(instanceCount);
                }
            }
        #endif
    }

    void SceneRenderer::fillDebugLinesVertexBuffer() {
        auto vertices = std::vector<float>();

        for (const auto& line: debugLinesDrawInfoQueue) {
            vertices.push_back(line.from.x);
            vertices.push_back(line.from.y);
            vertices.push_back(line.from.z);

            vertices.push_back(line.color.x);
            vertices.push_back(line.color.y);
            vertices.push_back(line.color.z);

            vertices.push_back(line.to.x);
            vertices.push_back(line.to.y);
            vertices.push_back(line.to.z);

            vertices.push_back(line.color.x);
            vertices.push_back(line.color.y);
            vertices.push_back(line.color.z);
        }

        debugLinesVAO->getVertexBuffer(0)->setSubData(0, vertices.data(), vertices.size() * sizeof(float));
    }

    void SceneRenderer::resetRenderingStats() {
        renderingStats = {};
    }
}
