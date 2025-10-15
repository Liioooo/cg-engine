#include "SceneRenderer.h"
#include "Asserts.h"
#include "Application.h"
#include "OpenGLTimer.h"
#include "OpenGLDebugGroup.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {
    SceneRenderer::SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight), invViewportWidth(1.0f / static_cast<float>(viewportWidth)), invViewportHeight(1.0f / static_cast<float>(viewportHeight)) {
        ubCameraData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBCameraData));
        ubLightData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBLightData));
        ubDirShadowData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBDirShadowData));
        ubScreenData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBScreenData));
        ubHBAOData = GraphicsObjectsFactory::createUniformBuffer(sizeof(UBHBAOData));

        transformOffsetPushConstant = GraphicsObjectsFactory::createPushConstants("pc_transformsOffset");
        transformOffsetPushConstant->init<TransformsOffsetPushConstants>();
        transformOffsetPushConstant->mapUniform(&TransformsOffsetPushConstants::transformsOffset, "transformsOffset");

        collidersPushConstants = GraphicsObjectsFactory::createPushConstants("pc_colliders");
        collidersPushConstants->init<CollidersPushConstants>();
        collidersPushConstants->mapUniform(&CollidersPushConstants::color, "color");
        collidersPushConstants->mapUniform(&CollidersPushConstants::transformsOffset, "transformsOffset");

        DescriptorSetSpecification environmentMapDescriptorSetSpec{};
        environmentMapDescriptorSetSpec.textureCubeBindings = {
            {5, Renderer::getBlackCubeTexture()},
            {6, Renderer::getBlackCubeTexture()}
        };
        environmentMapDescriptorSetBlack = GraphicsObjectsFactory::createDescriptorSet();

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
            shadowMapRenderPassSpec.clearDepthAttachment = true;
            shadowMapRenderPassSpec.clearStencilBuffer = false;
            shadowMapRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

            dirShadowMapRenderPass = GraphicsObjectsFactory::createRenderPass(shadowMapRenderPassSpec);

            GraphicsPipelineSpecification dirShadowMapPipelineSpec{};
            dirShadowMapPipelineSpec.renderPass = dirShadowMapRenderPass;
            dirShadowMapPipelineSpec.engineShaderName = "dirShadowMap";
            dirShadowMapPipelineSpec.frontfaceCulling = false;
            dirShadowMapPipelineSpec.backfaceCulling = true;
            dirShadowMapPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;

            dirShadowMapPipeline = GraphicsObjectsFactory::createGraphicsPipeline(dirShadowMapPipelineSpec);

            FramebufferSpecification shadowMapFramebufferSpec{};
            shadowMapFramebufferSpec.renderPass = dirShadowMapRenderPass;
            shadowMapFramebufferSpec.height = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.width = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.depthAttachment.attachment = dirShadowMaps;

            dirShadowMapFramebuffer = GraphicsObjectsFactory::createFramebuffer(shadowMapFramebufferSpec);

            dirShadowMapTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification dirShadowMapDescriptorSetSpec{};
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
            gBufferRenderPassSpec.clearDepthAttachment = true;
            gBufferRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};

            gBufferRenderPass = GraphicsObjectsFactory::createRenderPass(gBufferRenderPassSpec);

            GraphicsPipelineSpecification gBufferPipelineSpec{};
            gBufferPipelineSpec.renderPass = gBufferRenderPass;
            gBufferPipelineSpec.engineShaderName = "gBuffer";
            gBufferPipelineSpec.depthCompareOperator = DepthCompareOperator::Less;
            gBufferPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;

            gBufferPipeline = GraphicsObjectsFactory::createGraphicsPipeline(gBufferPipelineSpec);

            FramebufferSpecification gBufferFramebufferSpec{};
            gBufferFramebufferSpec.height = viewportHeight;
            gBufferFramebufferSpec.width = viewportWidth;
            gBufferFramebufferSpec.renderPass = gBufferRenderPass;
            gBufferFramebufferSpec.colorAttachments = {
                {gBufferAlbedoRoughnessAttachment},
                {gBufferEmissionMetallicAttachment},
                {gBufferWorldNormalsAttachment},
                {gBufferViewNormalsAttachment}
            };
            gBufferFramebufferSpec.depthAttachment.attachment = gBufferDepthAttachment;
            gBufferFramebufferSpec.depthAttachment.allLayers = true;

            gBufferFramebuffer = GraphicsObjectsFactory::createFramebuffer(gBufferFramebufferSpec);

            gBufferTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification gBufferDescriptorSetSpec{};
            gBufferDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData}
            };
            gBufferDescriptorSetSpec.ssboBindings = {
                    {1, gBufferTransformsBuffer}
            };

            gBufferDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(gBufferDescriptorSetSpec);
        }
        /*
        {
            RenderPassSpecification customShaderDeferredRenderPassSpec;
            customShaderDeferredRenderPassSpec.framebuffer = gBufferRenderPass.getSpecification().framebuffer;
            customShaderDeferredRenderPassSpec.usingExistingFramebuffer = true;
            customShaderDeferredRenderPassSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            customShaderDeferredRenderPassSpec.depthWrite = true;
            customShaderDeferredRenderPassSpec.depthTest = true;
            customShaderDeferredRenderPassSpec.clearColorBuffer = false;
            customShaderDeferredRenderPassSpec.clearDepthBuffer = false;
            customShaderDeferredRenderPassSpec.clearStencilBuffer = false;

            customShaderDeferredRenderPass = RenderPass(std::move(customShaderDeferredRenderPassSpec));
        }
         */
        {
            glm::uvec2 quarterSize = (glm::uvec2(viewportWidth, viewportHeight) + 3u) / 4u;

            hbaoUVOffsetPushConstants = GraphicsObjectsFactory::createPushConstants("pc_uvOffset");
            hbaoUVOffsetPushConstants->init<HbaoUVOffsetPushConstants>();
            hbaoUVOffsetPushConstants->mapUniform(&HbaoUVOffsetPushConstants::uvOffset, "uvOffset");

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
            hbaoDeinterleavingRenderPassSpec.clearDepthAttachment = false;
            hbaoDeinterleavingRenderPassSpec.clearColorAttachments = true;
            hbaoDeinterleavingRenderPassSpec.clearColor = {1.0f, 1.0f, 1.0f, 1.0f};

            hbaoDeinterleavingRenderPass = GraphicsObjectsFactory::createRenderPass(hbaoDeinterleavingRenderPassSpec);

            GraphicsPipelineSpecification hbaoDeinterleavingPipelineSpec{};
            hbaoDeinterleavingPipelineSpec.renderPass = hbaoDeinterleavingRenderPass;
            hbaoDeinterleavingPipelineSpec.engineShaderName = "hbaoDeinterleaving";
            hbaoDeinterleavingPipelineSpec.depthWrite = false;
            hbaoDeinterleavingPipelineSpec.depthTest = false;
            hbaoDeinterleavingPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();

            hbaoDeinterleavingPipeline = GraphicsObjectsFactory::createGraphicsPipeline(hbaoDeinterleavingPipelineSpec);

            FramebufferSpecification hbaoDeinterleavingFramebufferSpec0{};
            hbaoDeinterleavingFramebufferSpec0.width = quarterSize.x;
            hbaoDeinterleavingFramebufferSpec0.height = quarterSize.y;
            hbaoDeinterleavingFramebufferSpec0.renderPass = hbaoDeinterleavingRenderPass;
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
            hbaoDeinterleavingFramebufferSpec1.renderPass = hbaoDeinterleavingRenderPass;
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
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings.resize(1);
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].attachment = gBufferDepthAttachment;
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoDeinterleavingDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            hbaoDeinterleavingDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData},
                    {3, ubScreenData}
            };

            hbaoDeinterleavingDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(hbaoDeinterleavingDescriptorSetSpec);

            ComputePipelineSpecification hbaoComputePipelineSpec{};
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
            hbaoResultSpec.textureWrap = TextureWrap::Clamp;
            hbaoResultSpec.mipMapFiltering = MipMapFiltering::Nearest;

            hbaoResult = GraphicsObjectsFactory::createAttachment(hbaoResultSpec);

            DescriptorSetSpecification hbaoComputeDescriptorSetSpec{};
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
            hbaoDeinterleavingAttachmentSpec.layerCount = 1;
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
            hbaoReinterleavingRenderPassSpec.clearDepthAttachment = false;
            hbaoReinterleavingRenderPassSpec.clearStencilBuffer = false;

            hbaoReinterleavingRenderPass = GraphicsObjectsFactory::createRenderPass(hbaoReinterleavingRenderPassSpec);

            GraphicsPipelineSpecification hbaoReinterleavingPipelineSpec{};
            hbaoReinterleavingPipelineSpec.renderPass = hbaoReinterleavingRenderPass;
            hbaoReinterleavingPipelineSpec.engineShaderName = "hbaoReinterleaving";
            hbaoReinterleavingPipelineSpec.frontfaceCulling = false;
            hbaoReinterleavingPipelineSpec.backfaceCulling = false;
            hbaoReinterleavingPipelineSpec.depthWrite = false;
            hbaoReinterleavingPipelineSpec.depthTest = false;
            hbaoReinterleavingPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();

            hbaoReinterleavingPipeline = GraphicsObjectsFactory::createGraphicsPipeline(hbaoReinterleavingPipelineSpec);

            FramebufferSpecification hbaoReinterleavingFramebufferSpec{};
            hbaoReinterleavingFramebufferSpec.width = viewportWidth;
            hbaoReinterleavingFramebufferSpec.height = viewportHeight;
            hbaoReinterleavingFramebufferSpec.renderPass = hbaoReinterleavingRenderPass;
            hbaoReinterleavingFramebufferSpec.colorAttachments = {
                    {hbaoReinterleavingAttachment}
            };

            hbaoReinterleavingFramebuffer = GraphicsObjectsFactory::createFramebuffer(hbaoReinterleavingFramebufferSpec);

            DescriptorSetSpecification hbaoReinterleavingDescriptorSetSpec{};
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings.resize(1);
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].attachment = hbaoResult;
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoReinterleavingDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;

            hbaoReinterleavingDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(hbaoReinterleavingDescriptorSetSpec);

            AttachmentSpecification hbaoBlurAttachmentSpec{};
            hbaoBlurAttachmentSpec.width = viewportWidth;
            hbaoBlurAttachmentSpec.height = viewportHeight;
            hbaoBlurAttachmentSpec.type = AttachmentType::RG16F;

            hbaoBlurAttachment0 = GraphicsObjectsFactory::createAttachment(hbaoBlurAttachmentSpec);
            hbaoBlurAttachment1 = GraphicsObjectsFactory::createAttachment(hbaoBlurAttachmentSpec);

            RenderPassSpecification hbaoBlurRenderPassSpec0{};
            hbaoBlurRenderPassSpec0.clearColorAttachments = true;
            hbaoBlurRenderPassSpec0.clearDepthAttachment = false;
            hbaoBlurRenderPassSpec0.clearColor = {1.0f, 1.0f, 1.0f, 0.0f};
            hbaoBlurRenderPassSpec0.clearStencilBuffer = false;
            hbaoBlurRenderPass0 = GraphicsObjectsFactory::createRenderPass(hbaoBlurRenderPassSpec0);

            RenderPassSpecification hbaoBlurRenderPassSpec1{};
            hbaoBlurRenderPassSpec1.clearColorAttachments = false;
            hbaoBlurRenderPassSpec1.clearDepthAttachment = false;
            hbaoBlurRenderPassSpec1.clearStencilBuffer = false;
            hbaoBlurRenderPass1 = GraphicsObjectsFactory::createRenderPass(hbaoBlurRenderPassSpec1);

            GraphicsPipelineSpecification hbaoBlurPipelineSpec0{};
            hbaoBlurPipelineSpec0.renderPass = hbaoBlurRenderPass0;
            hbaoBlurPipelineSpec0.engineShaderName = "hbaoBlur";
            hbaoBlurPipelineSpec0.frontfaceCulling = false;
            hbaoBlurPipelineSpec0.backfaceCulling = false;
            hbaoBlurPipelineSpec0.depthTest = false;
            hbaoBlurPipelineSpec0.depthWrite = false;

            hbaoBlurPipeline0 = GraphicsObjectsFactory::createGraphicsPipeline(hbaoBlurPipelineSpec0);

            GraphicsPipelineSpecification hbaoBlurPipelineSpec1{};
            hbaoBlurPipelineSpec1.renderPass = hbaoBlurRenderPass0;
            hbaoBlurPipelineSpec1.engineShaderName = "hbaoBlur";
            hbaoBlurPipelineSpec1.frontfaceCulling = false;
            hbaoBlurPipelineSpec1.backfaceCulling = false;
            hbaoBlurPipelineSpec1.depthTest = false;
            hbaoBlurPipelineSpec1.depthWrite = false;

            hbaoBlurPipeline1 = GraphicsObjectsFactory::createGraphicsPipeline(hbaoBlurPipelineSpec1);

            FramebufferSpecification hbaoBlurFramebufferSpec0{};
            hbaoBlurFramebufferSpec0.width = viewportWidth;
            hbaoBlurFramebufferSpec0.height = viewportHeight;
            hbaoBlurFramebufferSpec0.renderPass = hbaoBlurRenderPass0;
            hbaoBlurFramebufferSpec0.colorAttachments = {
                    {hbaoBlurAttachment0}
            };

            hbaoBlurFramebuffer0 = GraphicsObjectsFactory::createFramebuffer(hbaoBlurFramebufferSpec0);

            FramebufferSpecification hbaoBlurFramebufferSpec1{};
            hbaoBlurFramebufferSpec1.width = viewportWidth;
            hbaoBlurFramebufferSpec1.height = viewportHeight;
            hbaoBlurFramebufferSpec1.renderPass = hbaoBlurRenderPass1;
            hbaoBlurFramebufferSpec1.colorAttachments = {
                    {hbaoBlurAttachment1}
            };

            hbaoBlurFramebuffer1 = GraphicsObjectsFactory::createFramebuffer(hbaoBlurFramebufferSpec1);

            DescriptorSetSpecification hbaoBlurDescriptorSetSpec0{};
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings.resize(1);
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].attachment = hbaoReinterleavingAttachment;
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoBlurDescriptorSetSpec0.attachmentTextureBindings[0].allLayers = true;

            hbaoBlurDescriptorSet0 = GraphicsObjectsFactory::createDescriptorSet(hbaoBlurDescriptorSetSpec0);

            DescriptorSetSpecification hbaoBlurDescriptorSetSpec1{};
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings.resize(1);
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].attachment = hbaoBlurAttachment0;
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].bindingPoint = 0;
            hbaoBlurDescriptorSetSpec1.attachmentTextureBindings[0].allLayers = true;

            hbaoBlurDescriptorSet1 = GraphicsObjectsFactory::createDescriptorSet(hbaoBlurDescriptorSetSpec1);

            hbaoBlurPushConstants = GraphicsObjectsFactory::createPushConstants("pc_hbaoBlur");
            hbaoBlurPushConstants->init<HbaoBlurPushConstants>();
            hbaoBlurPushConstants->mapUniform(&HbaoBlurPushConstants::sharpness, "sharpness");
            hbaoBlurPushConstants->mapUniform(&HbaoBlurPushConstants::invResolutionDirection, "invResolutionDirection");
        }
        {
            RenderPassSpecification pbrRenderPassSpec{};
            pbrRenderPassSpec.clearColorAttachments = true;
            pbrRenderPassSpec.clearDepthAttachment = false;
            pbrRenderPassSpec.clearStencilBuffer = false;
            pbrRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            pbrRenderPassSpec.hasDepthStencilAttachment = false;
            pbrRenderPassSpec.colorAttachments = {AttachmentType::RGBA16F};

            pbrRenderPass = GraphicsObjectsFactory::createRenderPass(pbrRenderPassSpec);

            GraphicsPipelineSpecification pbrPipelineSpec{};
            pbrPipelineSpec.renderPass = pbrRenderPass;
            pbrPipelineSpec.engineShaderName = "pbr";
            pbrPipelineSpec.depthWrite = false;
            pbrPipelineSpec.depthTest = false;
            pbrPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();

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
            pbrFramebufferSpec.renderPass = pbrRenderPass;
            pbrFramebufferSpec.colorAttachments = {
                {pbrColorAttachment}
            };

            pbrFramebuffer = GraphicsObjectsFactory::createFramebuffer(pbrFramebufferSpec);

            DescriptorSetSpecification pbrDescriptorSetSpec{};
            pbrDescriptorSetSpec.attachmentTextureBindings.resize(6);
            pbrDescriptorSetSpec.attachmentTextureBindings[0].attachment = gBufferAlbedoRoughnessAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            pbrDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].attachment = gBufferEmissionMetallicAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].bindingPoint = 1;
            pbrDescriptorSetSpec.attachmentTextureBindings[1].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].attachment = gBufferWorldNormalsAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].bindingPoint = 2;
            pbrDescriptorSetSpec.attachmentTextureBindings[2].allLayers = true;
            pbrDescriptorSetSpec.attachmentTextureBindings[3].attachment = gBufferDepthAttachment;
            pbrDescriptorSetSpec.attachmentTextureBindings[3].bindingPoint = 3;
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

            pbrPushConstants = GraphicsObjectsFactory::createPushConstants("pc_pbr");
            pbrPushConstants->init<PbrPushConstants>();
            pbrPushConstants->mapUniform(&PbrPushConstants::environmentIntensity, "environmentIntensity");
        }
        /*
        {
            RenderPassSpecification customShaderForwardRenderPassSpec;
            customShaderForwardRenderPassSpec.framebuffer = pbrRenderPass.getSpecification().framebuffer;
            customShaderForwardRenderPassSpec.usingExistingFramebuffer = true;
            customShaderForwardRenderPassSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            customShaderForwardRenderPassSpec.depthWrite = true;
            customShaderForwardRenderPassSpec.depthTest = true;
            customShaderForwardRenderPassSpec.clearColorBuffer = false;
            customShaderForwardRenderPassSpec.clearDepthBuffer = false;
            customShaderForwardRenderPassSpec.clearStencilBuffer = false;

            customShaderForwardRenderPass = RenderPass(std::move(customShaderForwardRenderPassSpec));
        }
         */
        {
            RenderPassSpecification afterPbrRenderPassSpec{};
            afterPbrRenderPassSpec.clearColorAttachments = false;
            afterPbrRenderPassSpec.clearDepthAttachment = false;
            afterPbrRenderPassSpec.clearStencilBuffer = false;
            afterPbrRenderPassSpec.hasDepthStencilAttachment = true;
            afterPbrRenderPassSpec.depthAttachmentFormat = gBufferDepthAttachment->getDepthAttachmentFormat();
            afterPbrRenderPassSpec.colorAttachments = {
                    pbrColorAttachment->getType()
            };

            afterPbrRenderPass = GraphicsObjectsFactory::createRenderPass(afterPbrRenderPassSpec);

            FramebufferSpecification afterPbrFramebufferSpec{};
            afterPbrFramebufferSpec.renderPass = afterPbrRenderPass;
            afterPbrFramebufferSpec.width = viewportWidth;
            afterPbrFramebufferSpec.height = viewportHeight;
            afterPbrFramebufferSpec.colorAttachments = {
                    {pbrColorAttachment}
            };
            afterPbrFramebufferSpec.depthAttachment.attachment = gBufferDepthAttachment;
            afterPbrFramebufferSpec.depthAttachment.allLayers = true;

            afterPbrFramebuffer = GraphicsObjectsFactory::createFramebuffer(afterPbrFramebufferSpec);
        }
        {
            GraphicsPipelineSpecification skyboxPipelineSpec{};
            skyboxPipelineSpec.renderPass = afterPbrRenderPass;
            skyboxPipelineSpec.engineShaderName = "skybox";
            skyboxPipelineSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            skyboxPipelineSpec.depthTest = true;
            skyboxPipelineSpec.vertexInputLayout = Renderer::getUnitCubeVertexInputLayout();

            skyboxPipeline = GraphicsObjectsFactory::createGraphicsPipeline(skyboxPipelineSpec);

            skyboxPushConstants = GraphicsObjectsFactory::createPushConstants("pc_skybox");
            skyboxPushConstants->init<SkyboxPushConstants>();
            skyboxPushConstants->mapUniform(&SkyboxPushConstants::intensity, "intensity");
            skyboxPushConstants->mapUniform(&SkyboxPushConstants::lod, "lod");
        }
        {
            GraphicsPipelineSpecification physicsCollidersPipelineSpec;
            physicsCollidersPipelineSpec.renderPass = afterPbrRenderPass;
            physicsCollidersPipelineSpec.engineShaderName = "colliders";
            physicsCollidersPipelineSpec.depthTest = false;
            physicsCollidersPipelineSpec.depthWrite = false;
            physicsCollidersPipelineSpec.wireframe = true;
            physicsCollidersPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;

            physicsCollidersPipeline = GraphicsObjectsFactory::createGraphicsPipeline(physicsCollidersPipelineSpec);

            physicsCollidersTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification physicsCollidersDescriptorSetSpec{};
            physicsCollidersDescriptorSetSpec.uboBindings = {
                    {0, ubCameraData}
            };
            physicsCollidersDescriptorSetSpec.ssboBindings = {
                    {1, physicsCollidersTransformsBuffer}
            };

            physicsCollidersDescriptorSet = GraphicsObjectsFactory::createDescriptorSet(physicsCollidersDescriptorSetSpec);
        }
        {
            GraphicsPipelineSpecification boundingBoxPipelineSpec;
            boundingBoxPipelineSpec.renderPass = afterPbrRenderPass;
            boundingBoxPipelineSpec.engineShaderName = "colliders";
            boundingBoxPipelineSpec.depthTest = true;
            boundingBoxPipelineSpec.depthWrite = false;
            boundingBoxPipelineSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            boundingBoxPipelineSpec.wireframe = true;
            boundingBoxPipelineSpec.backfaceCulling = false;
            boundingBoxPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;

            boundingBoxPipeline = GraphicsObjectsFactory::createGraphicsPipeline(boundingBoxPipelineSpec);

            boundingBoxTransformsBuffer = GraphicsObjectsFactory::createShaderStorageBuffer(MAX_OBJECTS * sizeof(glm::mat4));

            DescriptorSetSpecification boundingBoxDescriptorSetSpec{};
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
            mormalsDebugPipelineSpec.renderPass = afterPbrRenderPass;
            mormalsDebugPipelineSpec.engineShaderName = "normalsVisualize";
            mormalsDebugPipelineSpec.depthTest = true;
            mormalsDebugPipelineSpec.depthWrite = false;
            mormalsDebugPipelineSpec.vertexInputLayout = MeshProps::DEFAULT_VERT_BUFF_LAYOUTS;

            normalsDebugPipeline = GraphicsObjectsFactory::createGraphicsPipeline(mormalsDebugPipelineSpec);
        }
        {
            debugLinesVAO = GraphicsObjectsFactory::createVertexArrayObject();
            auto* linesVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(MAX_DEBUG_LINES * 2 * sizeof(float) * 6, VertexBufferUsage::Dynamic);
            linesVertexBuffer->setLayout({{ShaderDataType::Float3, false}, {ShaderDataType::Float3, false}});
            debugLinesVAO->addVertexBuffer(linesVertexBuffer);

            GraphicsPipelineSpecification debugLinesPipelineSpec{};
            debugLinesPipelineSpec.engineShaderName = "lines";
            debugLinesPipelineSpec.depthTest = true;
            debugLinesPipelineSpec.depthWrite = false;
            debugLinesPipelineSpec.drawMode = DrawMode::Lines;

            debugLinesPipeline = GraphicsObjectsFactory::createGraphicsPipeline(debugLinesPipelineSpec);

            DescriptorSetSpecification debugLinesDescriptorSetSpec{};
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
            bloomDownSamplePassSpec.clearDepthAttachment = false;
            bloomDownSamplePassSpec.hasDepthStencilAttachment = false;
            bloomDownSamplePassSpec.colorAttachments = { AttachmentType::RGBA16F };

            bloomDownSamplePass = GraphicsObjectsFactory::createRenderPass(bloomDownSamplePassSpec);

            GraphicsPipelineSpecification bloomDownsamplePipelineSpec{};
            bloomDownsamplePipelineSpec.renderPass = bloomDownSamplePass;
            bloomDownsamplePipelineSpec.engineShaderName = "bloomDownSample";
            bloomDownsamplePipelineSpec.depthTest = false;
            bloomDownsamplePipelineSpec.depthWrite = false;

            bloomDownsamplePipeline = GraphicsObjectsFactory::createGraphicsPipeline(bloomDownsamplePipelineSpec);

            for (size_t i = 0; i < bloomDownsampleFramebuffers.size(); i++) {
                FramebufferSpecification bloomFramebufferSpec{};
                bloomFramebufferSpec.width = bloomAttachments[i]->getWidth();
                bloomFramebufferSpec.height = bloomAttachments[i]->getHeight();
                bloomFramebufferSpec.renderPass = bloomDownSamplePass;
                bloomFramebufferSpec.colorAttachments = {
                        {bloomAttachments[i]}
                };
                bloomDownsampleFramebuffers[i] = GraphicsObjectsFactory::createFramebuffer(bloomFramebufferSpec);
            }

            RenderPassSpecification bloomUpSamplePassSpec;
            bloomUpSamplePassSpec.clearColorAttachments = false;
            bloomUpSamplePassSpec.clearDepthAttachment = false;
            bloomUpSamplePassSpec.hasDepthStencilAttachment = false;
            bloomUpSamplePassSpec.colorAttachments = { AttachmentType::RGBA16F };

            bloomUpSamplePass = GraphicsObjectsFactory::createRenderPass(bloomUpSamplePassSpec);

            GraphicsPipelineSpecification bloomUpsamplePipelineSpec{};
            bloomUpsamplePipelineSpec.renderPass = bloomUpSamplePass;
            bloomUpsamplePipelineSpec.engineShaderName = "bloomUpSample";
            bloomUpsamplePipelineSpec.depthTest = false;
            bloomUpsamplePipelineSpec.depthWrite = false;
            bloomUpsamplePipelineSpec.useBlending = true;
            bloomUpsamplePipelineSpec.blendingEquation = BlendingEquation::Add;
            bloomUpsamplePipelineSpec.srcBlendingFunction = BlendingFunction::One;
            bloomUpsamplePipelineSpec.destBlendingFunction = BlendingFunction::One;

            bloomUpsamplePipeline = GraphicsObjectsFactory::createGraphicsPipeline(bloomUpsamplePipelineSpec);

            for (size_t i = 0; i < bloomUpsampleFramebuffers.size(); i++) {
                FramebufferSpecification bloomFramebufferSpec{};
                bloomFramebufferSpec.width = bloomAttachments[i]->getWidth();
                bloomFramebufferSpec.height = bloomAttachments[i]->getHeight();
                bloomFramebufferSpec.renderPass = bloomUpSamplePass;
                bloomFramebufferSpec.colorAttachments = {
                        {bloomAttachments[i]}
                };
                bloomUpsampleFramebuffers[i] = GraphicsObjectsFactory::createFramebuffer(bloomFramebufferSpec);
            }

            bloomDownsamplePushConstants = GraphicsObjectsFactory::createPushConstants("pc_bloomDownsample");
            bloomDownsamplePushConstants->init<BloomDownsamplePushConstants>();
            bloomDownsamplePushConstants->mapUniform(&BloomDownsamplePushConstants::useThreshold, "useThreshold");

            DescriptorSetSpecification bloomDescriptorSetSpec0{};
            bloomDescriptorSetSpec0.attachmentTextureBindings.resize(1);
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].attachment = pbrColorAttachment;
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].bindingPoint = 0;
            bloomDescriptorSetSpec0.attachmentTextureBindings[0].allLayers = true;

            bloomDescriptorSets[0] = GraphicsObjectsFactory::createDescriptorSet(bloomDescriptorSetSpec0);

            for (size_t i = 1; i < bloomDescriptorSets.size(); i++) {
                DescriptorSetSpecification bloomDescriptorSetSpec{};
                bloomDescriptorSetSpec.attachmentTextureBindings.resize(1);
                bloomDescriptorSetSpec.attachmentTextureBindings[0].attachment = bloomAttachments[i - 1];
                bloomDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
                bloomDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;

                bloomDescriptorSets[i] = GraphicsObjectsFactory::createDescriptorSet(bloomDescriptorSetSpec);
            }
        }
        {
            GraphicsPipelineSpecification screenPipelineSpec;
            screenPipelineSpec.renderPass = Renderer::getSwapChainRenderPass();
            screenPipelineSpec.engineShaderName = "screen";
            screenPipelineSpec.depthTest = false;
            screenPipelineSpec.depthWrite = false;
            screenPipelineSpec.vertexInputLayout = Renderer::getUnitQuadVertexInputLayout();

            screenPipeline = GraphicsObjectsFactory::createGraphicsPipeline(screenPipelineSpec);

            DescriptorSetSpecification screenDescriptorSetSpec{};
            screenDescriptorSetSpec.attachmentTextureBindings.resize(2);
            screenDescriptorSetSpec.attachmentTextureBindings[0].attachment = pbrColorAttachment;
            screenDescriptorSetSpec.attachmentTextureBindings[0].bindingPoint = 0;
            screenDescriptorSetSpec.attachmentTextureBindings[0].allLayers = true;
            screenDescriptorSetSpec.attachmentTextureBindings[1].attachment = bloomAttachments[0];
            screenDescriptorSetSpec.attachmentTextureBindings[1].bindingPoint = 1;
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

            uiCircleVAO = GraphicsObjectsFactory::createVertexArrayObject();
            auto* uiCircleVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiCircleVertex) * MAX_UI_VERTICES, VertexBufferUsage::Dynamic);
            uiCircleVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
            uiCircleVAO->addVertexBuffer(uiCircleVertexBuffer);
            uiCircleVAO->useExistingIndexBuffer(uiIndexBuffer);

            uiRectVAO = GraphicsObjectsFactory::createVertexArrayObject();
            auto* uiRectVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiRectVertex) * MAX_UI_VERTICES, VertexBufferUsage::Dynamic);
            uiRectVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float2, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
            uiRectVAO->addVertexBuffer(uiRectVertexBuffer);
            uiRectVAO->useExistingIndexBuffer(uiIndexBuffer);

            uiTextVAO = GraphicsObjectsFactory::createVertexArrayObject();
            auto* uiTextVertexBuffer = GraphicsObjectsFactory::createVertexBuffer(sizeof(UiTextVertex) * MAX_UI_VERTICES, VertexBufferUsage::Dynamic);
            uiTextVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}});
            uiTextVAO->addVertexBuffer(uiTextVertexBuffer);
            uiTextVAO->useExistingIndexBuffer(uiIndexBuffer);

            GraphicsPipelineSpecification uiCirclePipelineSpec;
            uiCirclePipelineSpec.renderPass = Renderer::getSwapChainRenderPass();
            uiCirclePipelineSpec.engineShaderName = "uiCircle";
            uiCirclePipelineSpec.depthTest = false;
            uiCirclePipelineSpec.depthWrite = false;
            uiCirclePipelineSpec.useBlending = true;
            uiCirclePipelineSpec.blendingEquation = BlendingEquation::Add;
            uiCirclePipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiCirclePipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiCirclePipelineSpec.vertexInputLayout = uiCircleVAO->getLayout();

            uiCirclePipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiCirclePipelineSpec);

            GraphicsPipelineSpecification uiRectPipelineSpec;
            uiRectPipelineSpec.renderPass = Renderer::getSwapChainRenderPass();
            uiRectPipelineSpec.engineShaderName = "uiRect";
            uiRectPipelineSpec.depthTest = false;
            uiRectPipelineSpec.depthWrite = false;
            uiRectPipelineSpec.useBlending = true;
            uiRectPipelineSpec.blendingEquation = BlendingEquation::Add;
            uiRectPipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiRectPipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiRectPipelineSpec.vertexInputLayout = uiRectVAO->getLayout();

            uiRectPipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiRectPipelineSpec);

            for (auto& item: uiDescriptorSets) {
                item = GraphicsObjectsFactory::createDescriptorSet();
            }
            for (auto& item: uiTextDescriptorSets) {
                item = GraphicsObjectsFactory::createDescriptorSet();
            }

            GraphicsPipelineSpecification uiTextPipelineSpec;
            uiTextPipelineSpec.renderPass = Renderer::getSwapChainRenderPass();
            uiTextPipelineSpec.engineShaderName = "uiText";
            uiTextPipelineSpec.depthTest = false;
            uiTextPipelineSpec.depthWrite = false;
            uiTextPipelineSpec.useBlending = true;
            uiTextPipelineSpec.blendingEquation = BlendingEquation::Add;
            uiTextPipelineSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiTextPipelineSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiTextPipelineSpec.vertexInputLayout = uiTextVAO->getLayout();

            uiTextPipeline = GraphicsObjectsFactory::createGraphicsPipeline(uiTextPipelineSpec);

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

//        boneTransformsBuffer = ShaderStorageBuffer();
//        boneTransformsBuffer.setData(nullptr, maxBones * maxAnimatedComponents * sizeof(glm::mat4));
//
//        skinningShader = ComputeShader("skinning");
    }

    SceneRenderer::~SceneRenderer() {
        delete hbaoDeinterleavingFramebuffers[0];
        delete hbaoDeinterleavingFramebuffers[1];
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


        SkyboxPushConstants skyboxPushConstantsData{};
        skyboxPushConstantsData.intensity = sceneEnvironment.environmentIntensity;
        skyboxPushConstantsData.lod = sceneEnvironment.environmentLod;
        skyboxPushConstants->setData(&skyboxPushConstantsData, sizeof(SkyboxPushConstants));

        currentSceneEnvironment.environmentIntensity = sceneEnvironment.environmentIntensity;
        currentSceneEnvironment.environmentMapDescriptorSet = sceneEnvironment.environmentMapDescriptorSet != nullptr ? sceneEnvironment.environmentMapDescriptorSet : environmentMapDescriptorSetBlack;
        currentSceneEnvironment.dirLightCastShadows = lightEnvironment.dirLightCastShadows && lightEnvironment.dirLightIntensity != 0.0f;

        pbrPushConstants->setData(&currentSceneEnvironment.environmentIntensity, sizeof(float));

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
        buildUiVertexBuffers();
        fillDebugLinesVertexBuffer();

        skinMeshes();
        shadowMapPass();
        gBufferPass();
        customShaderDeferredPass();

        if (applicationOptions.enableHBAO) {
            hbaoDeinterleavingPass();
            hbaoComputePass();
            hbaoReinterleavingPass();
            hbaoBlurPass();
        } else {
            Renderer::clearPass(hbaoBlurRenderPass0, hbaoBlurFramebuffer1);
        }

        pbrPass();
        customShaderForwardPass();

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

        Renderer::beginSwapChainRenderPass();
        screenPass();
        uiPass();
        Renderer::endRenderPass();

        skinningQueue.clear();

        drawCommandQueue.clear();
        meshTransforms.clear();

//        customShaderDeferredDrawCommandQueue.clear();
//        customShaderForwardDrawCommandQueue.clear();

        shadowMapDrawCommandQueue.clear();
        shadowMapMeshTransforms.clear();

        uiDrawInfoQueue.clear();

        #ifdef CG_ENABLE_DEBUG_FEATURES
            physicsCollidersDrawCommandQueue.clear();
            physicsCollidersMeshTransforms.clear();

            boundingBoxDrawCommandQueue.clear();
            boundingBoxMeshTransforms.clear();
        #endif

        debugLinesDrawInfoQueue.clear();

        activeRendering = false;
    }

    void SceneRenderer::submitMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* overrideMaterial, bool castShadows, bool enableCulling, const glm::mat4& transform, const std::vector<float>& lodDistances) {
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
                    const Material* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
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

    void SceneRenderer::submitAnimatedMesh(MeshVertices* mesh, const std::vector<uint32_t>& meshNodes, Material* overrideMaterial, bool castShadows, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, VertexArrayObject* skinnedVAO) {
//        CG_ASSERT(boneTransforms.size() <= maxBones, "Mesh contains to many bones")
//        CG_ASSERT(skinningQueue.size() < maxAnimatedComponents, "Cannot render that many AnimatedMeshRendererComponents")
//
//        uint32_t boneTransformOffset = skinningQueue.size() * maxBones * sizeof(glm::mat4);
//        boneTransformsBuffer.setSubData(boneTransformOffset, boneTransforms.data(), boneTransforms.size() * sizeof(glm::mat4));
//
//        SkinningInfo& skinningInfo = skinningQueue.emplace_back();
//        skinningInfo.originalVertexBuffer = mesh->getVAO()->getVertexBuffers()[0];
//        skinningInfo.skinnedVertexBuffer = skinnedVAO->getVertexBuffers()[0];
//        skinningInfo.boneInfluencesBuffer = &mesh->getBoneInfluencesBuffer();
//        skinningInfo.numVertices = mesh->getVertices().size();
//
//        auto& submeshes = mesh->getSubmeshes();
//
//        for (const auto& meshNodeIndex: meshNodes) {
//            const auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);
//
//            for (const auto& submeshIndex: meshNode.submeshIndices) {
//                const Submesh& submesh = submeshes.at(submeshIndex);
//                const Material* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
//                MeshKey mk = {skinnedVAO->getRendererId(), submeshIndex, material->getUuid().getUuid()};
//
//                meshTransforms[mk].emplace_back(transform);
//
//                DrawCommand& drawCommand = drawCommandQueue[mk];
//                drawCommand.vao = skinnedVAO;
//                drawCommand.material = material;
//                drawCommand.baseIndex = submesh.baseIndex;
//                drawCommand.baseVertex = submesh.baseVertex;
//                drawCommand.indexCount = submesh.indexCount;
//                drawCommand.instanceCount++;
//
//                if (castShadows) {
//                    shadowMapMeshTransforms[mk].emplace_back(transform);
//
//                    DrawCommand& shadowMapDrawCommand = shadowMapDrawCommandQueue[mk];
//                    shadowMapDrawCommand.vao = skinnedVAO;
//                    shadowMapDrawCommand.material = material;
//                    shadowMapDrawCommand.baseIndex = submesh.baseIndex;
//                    shadowMapDrawCommand.baseVertex = submesh.baseVertex;
//                    shadowMapDrawCommand.indexCount = submesh.indexCount;
//                    shadowMapDrawCommand.instanceCount++;
//                }
//            }
//        }
    }

//    void SceneRenderer::submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* material, bool enableCulling, const AABoundingBox* boundingBox, const glm::mat4& transform, CustomShader* shader, uint32_t instanceCount, CustomShaderRendererComponentRenderPassOptions& renderPassOptions, std::pair<ShaderStorageBuffer*, ShaderStorageBuffer*> instanceBuffers, const std::vector<float>& lodDistances) {
//        if (enableCulling && boundingBox != nullptr && !cameraFrustum.testAABoundingBoxInFrustum(*boundingBox, transform)) {
//            return;
//        }
//
//        auto& submeshes = mesh->getSubmeshes();
//
//        for (const auto& meshNodeIndex: meshNodes) {
//            const auto* meshNode = &mesh->getMeshNodes().at(meshNodeIndex);
//
//            glm::mat4 finalTransform = transform * meshNode->transform;
//
//            if (!meshNode->lodMeshNodes.empty()) {
//                meshNode = &mesh->getMeshNodes().at(meshNode->lodMeshNodes[findCorrectLodIndex(lodDistances, finalTransform, meshNode->lodMeshNodes.size())]);
//            }
//
//            bool isInCameraFrustum = !enableCulling || boundingBox != nullptr || cameraFrustum.testAABoundingBoxInFrustum(meshNode->aaBoundingBox, finalTransform);
//
//            if (isInCameraFrustum) {
//                for (const auto& submeshIndex: meshNode->submeshIndices) {
//                    const Submesh& submesh = submeshes.at(submeshIndex);
//
//                    if (shader->isForward()) {
//                        CustomShaderDrawCommand& drawCommand = customShaderForwardDrawCommandQueue[shader].emplace_back();
//                        drawCommand.instanceCount = instanceCount;
//                        drawCommand.vao = mesh->getVAO();
//                        drawCommand.material = material;
//                        drawCommand.baseIndex = submesh.baseIndex;
//                        drawCommand.baseVertex = submesh.baseVertex;
//                        drawCommand.indexCount = submesh.indexCount;
//                        drawCommand.transform = finalTransform;
//                        drawCommand.renderPassOptions = renderPassOptions;
//                        drawCommand.instanceBuffers = instanceBuffers;
//                    } else {
//                        CustomShaderDrawCommand& drawCommand = customShaderDeferredDrawCommandQueue[shader].emplace_back();
//                        drawCommand.instanceCount = instanceCount;
//                        drawCommand.vao = mesh->getVAO();
//                        drawCommand.material = material;
//                        drawCommand.baseIndex = submesh.baseIndex;
//                        drawCommand.baseVertex = submesh.baseVertex;
//                        drawCommand.indexCount = submesh.indexCount;
//                        drawCommand.transform = finalTransform;
//                        drawCommand.renderPassOptions = renderPassOptions;
//                        drawCommand.instanceBuffers = instanceBuffers;
//                    }
//                }
//            }
//        }
//    }

    void SceneRenderer::submitUiElements(const std::unordered_map<std::string, UiElement*>& uiElements) {
        for (const auto& [_, element]: uiElements) {
            UiDrawInfo& drawInfo = uiDrawInfoQueue[element->getZIndex()];

            if (element->getType() == UIElementType::Circle) {
                auto* circleElement = dynamic_cast<UiCircle*>(element);
                const auto* texture = circleElement->getTexture().get();
                float textureIndex = findDrawInfoTextureIndex(drawInfo, texture);

                for (const auto& v: element->getVertices()) {
                    UiCircleVertex& vertex = drawInfo.circleVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.fillColor = circleElement->getFillColor();
                    vertex.width = circleElement->getWidth();
                    vertex.lineColor = circleElement->getLineColor();
                    vertex.lineWidth = circleElement->getLineWidth();
                    vertex.textureIndex = textureIndex;
                }
                drawInfo.circleIndexCount += 6;
            } else if (element->getType() == UIElementType::Rect) {
                auto* rectElement = dynamic_cast<UiRect*>(element);
                const auto* texture = rectElement->getTexture().get();
                float textureIndex = findDrawInfoTextureIndex(drawInfo, texture);

                for (const auto& v: element->getVertices()) {
                    UiRectVertex& vertex = drawInfo.rectVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.fillColor = rectElement->getFillColor();
                    vertex.lineColor = rectElement->getLineColor();
                    vertex.size = rectElement->getSize();
                    vertex.lineWidth = rectElement->getLineWidth();
                    vertex.textureIndex = textureIndex;
                }
                drawInfo.rectIndexCount += 6;
            } else if (element->getType() == UIElementType::Text) {
                auto* textElement = dynamic_cast<UiText*>(element);
                const auto* fontAtlas = textElement->getFontAtlas();

                float fontAtlasIndex = -1;
                for (uint32_t i = 0; i < drawInfo.filledFontAtlases; i++) {
                    if (drawInfo.fontAtlases[i] == fontAtlas) {
                        fontAtlasIndex = static_cast<float>(i);
                        break;
                    }
                }
                if (fontAtlasIndex < 0.0f) {
                    fontAtlasIndex = static_cast<float>(drawInfo.filledFontAtlases);
                    drawInfo.fontAtlases[drawInfo.filledFontAtlases] = fontAtlas;
                    drawInfo.filledFontAtlases++;
                }

                for (const auto& v: element->getVertices()) {
                    UiTextVertex& vertex = drawInfo.textVertices.emplace_back();
                    vertex.posUV = v;
                    vertex.color = textElement->getColor();
                    vertex.fontAtlasIndex = fontAtlasIndex;
                }

                drawInfo.textIndexCount += textElement->getNumIndices();
            }

            CG_ASSERT(drawInfo.circleIndexCount <= MAX_UI_INDICES, "Cannot render that many UICircles")
            CG_ASSERT(drawInfo.rectIndexCount <= MAX_UI_INDICES, "Cannot render that many UIRects")
            CG_ASSERT(drawInfo.textIndexCount <= MAX_UI_INDICES, "Cannot render that many UIText")
            CG_ASSERT(drawInfo.filledTextureSlots < drawInfo.textureSlots.size(), "Cannot render that many different Textures on a single z-index")
            CG_ASSERT(drawInfo.filledFontAtlases < drawInfo.fontAtlases.size(), "Cannot render that many different Fonts on a single z-index")
            CG_ASSERT(uiDrawInfoQueue.size() < MAX_UI_Z_LAYERS, "Cannot render that many different z-indices")
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

    const CameraFrustum& SceneRenderer::getCamaraFrustum() const {
        return cameraFrustum;
    }

    const RenderingStats& SceneRenderer::getRenderingStats() {
        return renderingStats;
    }

    void SceneRenderer::skinMeshes() {
//        CG_GPU_DEBUG_GROUP("SkinMeshes")
//        CG_GPU_TIME_FN(&renderingStats.skinMeshesTimer)
//
//        skinningShader.bind();
//        boneTransformsBuffer.bind(2);
//
//        for (uint32_t i = 0; i < skinningQueue.size(); i++) {
//            skinningQueue[i].originalVertexBuffer->bindAsSSBO(3);
//            skinningQueue[i].skinnedVertexBuffer->bindAsSSBO(4);
//            skinningQueue[i].boneInfluencesBuffer->bind(1);
//
//            skinningShader.setInt("u_ComponentIndex", i);
//            skinningShader.dispatch((skinningQueue[i].numVertices / 32) + 1, 1, 1);
//            skinningShader.waitForMemoryBarrier({MemoryBarrierBit::All});
//        }
    }

    void SceneRenderer::shadowMapPass() {
        CG_GPU_DEBUG_GROUP("ShadowMapPass")
        CG_GPU_TIME_FN(&renderingStats.shadowMapTimer)

        if (!currentSceneEnvironment.dirLightCastShadows) {
            Renderer::clearPass(dirShadowMapRenderPass, dirShadowMapFramebuffer);
            return;
        }

        Renderer::beginRenderPass(dirShadowMapRenderPass, dirShadowMapFramebuffer);
        Renderer::bindGraphicsPipeline(dirShadowMapPipeline);
        Renderer::bindDescriptorSet(dirShadowMapDescriptorSet, 0);

        for (const auto [mk, command]: shadowMapDrawCommandQueue) {
            transformOffsetPushConstant->setData(&command.transformsBufferOffset, sizeof(int));
            Renderer::setPushConstants({transformOffsetPushConstant}, 1);
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
            transformOffsetPushConstant->setData(&command.transformsBufferOffset, sizeof(int));
            Renderer::setPushConstants({transformOffsetPushConstant, command.material->getPushConstants()}, 2);
            Renderer::bindDescriptorSet(command.material->getDescriptorSet(), 1);
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoDeinterleavingPass() {
        CG_GPU_DEBUG_GROUP("HBAODeinterleavingPass")
        CG_GPU_TIME_FN(&renderingStats.hbaoDeinterleavingTimer)

        int uvOffset = 0;

        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass, hbaoDeinterleavingFramebuffers[0]);
        Renderer::bindGraphicsPipeline(hbaoDeinterleavingPipeline);
        Renderer::bindDescriptorSet(hbaoDeinterleavingDescriptorSet, 0);
        hbaoUVOffsetPushConstants->setData(&uvOffset, sizeof(int));
        Renderer::setPushConstants({hbaoUVOffsetPushConstants}, 1);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        uvOffset = 1;

        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass, hbaoDeinterleavingFramebuffers[1]);
        Renderer::bindGraphicsPipeline(hbaoDeinterleavingPipeline);
        Renderer::bindDescriptorSet(hbaoDeinterleavingDescriptorSet, 0);
        hbaoUVOffsetPushConstants->setData(&uvOffset, sizeof(int));
        Renderer::setPushConstants({hbaoUVOffsetPushConstants}, 1);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoComputePass() {
        CG_GPU_DEBUG_GROUP("HBAOComputePass")
        CG_GPU_TIME_FN(&renderingStats.hbaoComputeTimer)

        Renderer::bindComputePipeline(hbaoComputePipeline);
        Renderer::bindDescriptorSet(hbaoComputeDescriptorSet, 0);
        Renderer::dispatchCompute(hbaoWorkGroupSize.x, hbaoWorkGroupSize.y, hbaoWorkGroupSize.z);
        Renderer::transitionImageLayoutFromComputeToShaderReadOnly(hbaoResult, ShaderStage::Fragment);
    }

    void SceneRenderer::hbaoReinterleavingPass() {
        CG_GPU_DEBUG_GROUP("HBAOReinterleavingPass")
        CG_GPU_TIME_FN(&renderingStats.hbaoReinterleavingTimer)

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

        Renderer::beginRenderPass(hbaoBlurRenderPass0, hbaoBlurFramebuffer0);
        Renderer::bindGraphicsPipeline(hbaoBlurPipeline0);
        Renderer::bindDescriptorSet(hbaoBlurDescriptorSet0, 0);
        hbaoBlurPushConstants->setData(&pc, sizeof(HbaoBlurPushConstants));
        Renderer::setPushConstants({hbaoBlurPushConstants}, 1);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        Renderer::beginRenderPass(hbaoBlurRenderPass1, hbaoBlurFramebuffer1);
        Renderer::bindGraphicsPipeline(hbaoBlurPipeline1);
        Renderer::bindDescriptorSet(hbaoBlurDescriptorSet1, 0);
        pc.invResolutionDirection = glm::vec2(0.0f, invViewportHeight);
        hbaoBlurPushConstants->setData(&pc, sizeof(HbaoBlurPushConstants));
        Renderer::setPushConstants({hbaoBlurPushConstants}, 1);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::pbrPass() {
        CG_GPU_DEBUG_GROUP("PBRPass")
        CG_GPU_TIME_FN(&renderingStats.pbrTimer)

        Renderer::beginRenderPass(pbrRenderPass, pbrFramebuffer);
        Renderer::bindGraphicsPipeline(pbrPipeline);
        Renderer::bindDescriptorSet(pbrDescriptorSet, 0);
        Renderer::bindDescriptorSet(currentSceneEnvironment.environmentMapDescriptorSet, 1);
        Renderer::setPushConstants({pbrPushConstants}, 1);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();
    }

    void SceneRenderer::customShaderDeferredPass() {
//        CG_GPU_DEBUG_GROUP("CustomShaderDeferredPass")
//        CG_GPU_TIME_FN(&renderingStats.customShaderDeferredTimer)
//
//        Renderer::beginRenderPass(customShaderDeferredRenderPass, true);
//
//        const Material* lastUsedMaterial = nullptr;
//
//        for (const auto& [shader, commands]: customShaderDeferredDrawCommandQueue) {
//            shader->bind();
//
//            for (const auto& command: commands) {
//                shader->setMat4("u_Transform", command.transform);
//
//                if (command.instanceBuffers.first != nullptr) {
//                    command.instanceBuffers.first->bind(5);
//                }
//                if (command.instanceBuffers.second != nullptr) {
//                    command.instanceBuffers.second->bind(6);
//                }
//
//                const Material* material = command.material != nullptr ? command.material : &emptyMaterial;
//
//                Renderer::setFaceCulling(command.renderPassOptions.backfaceCulling, command.renderPassOptions.frontfaceCulling);
//                Renderer::setTesselationPatchSize(command.renderPassOptions.tesselationPatchSize);
//
//                if (material != lastUsedMaterial) {
//                    material->uploadToShader(*shader);
//                }
//                lastUsedMaterial = material;
//
//                Renderer::executeCustomShaderDrawCommand(*command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount, command.renderPassOptions.tesselationPatchSize);
//            }
//        }
//
//        Renderer::endRenderPass();
    }

    void SceneRenderer::customShaderForwardPass() {
//        CG_GPU_DEBUG_GROUP("CustomShaderForwardPass")
//        CG_GPU_TIME_FN(&renderingStats.customShaderForwardTimer)
//
//        Renderer::beginRenderPass(customShaderForwardRenderPass, true);
//
//        bool lastCommandUseDirShadowMappingData = false;
//        bool lastCommandUseEnvironmentMappingData = false;
//
//        const Material* lastUsedMaterial = nullptr;
//
//        for (const auto& [shader, commands]: customShaderForwardDrawCommandQueue) {
//            shader->bind();
//
//            for (const auto& command: commands) {
//                shader->setMat4("u_Transform", command.transform);
//
//                if (command.instanceBuffers.first != nullptr) {
//                    command.instanceBuffers.first->bind(5);
//                }
//                if (command.instanceBuffers.second != nullptr) {
//                    command.instanceBuffers.second->bind(6);
//                }
//
//                if (command.renderPassOptions.useDirShadowMappingData && !lastCommandUseDirShadowMappingData) {
//                    shader->setTexture(dirShadowMaps.getRendererId(), 8);
//                }
//                lastCommandUseDirShadowMappingData = command.renderPassOptions.useDirShadowMappingData;
//
//                if (command.renderPassOptions.useEnvironmentMappingData && !lastCommandUseEnvironmentMappingData) {
//                    shader->setTexture(currentSceneEnvironment.irradianceMapId, 5);
//                    shader->setTexture(currentSceneEnvironment.prefilterMapId, 6);
//                    shader->setTexture(Renderer::getBrdfLUTTexture().getRendererId(), 7);
//                    shader->setFloat("u_EnvironmentIntensity", currentSceneEnvironment.environmentIntensity);
//                }
//                lastCommandUseEnvironmentMappingData = command.renderPassOptions.useEnvironmentMappingData;
//
//                const Material* material = command.material != nullptr ? command.material : &emptyMaterial;
//
//                Renderer::setFaceCulling(command.renderPassOptions.backfaceCulling, command.renderPassOptions.frontfaceCulling);
//                Renderer::setBlending(command.renderPassOptions.useBlending, command.renderPassOptions.blendingEquation, command.renderPassOptions.srcBlendingFunction, command.renderPassOptions.destBlendingFunction);
//                Renderer::setWireframe(command.renderPassOptions.wireframe);
//                Renderer::setTesselationPatchSize(command.renderPassOptions.tesselationPatchSize);
//
//                if (material != lastUsedMaterial) {
//                    material->uploadToShader(*shader);
//                }
//                lastUsedMaterial = material;
//
//                Renderer::executeCustomShaderDrawCommand(*command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount, command.renderPassOptions.tesselationPatchSize);
//            }
//        }
//
//        Renderer::endRenderPass();
    }

    void SceneRenderer::skyboxPass() {
        CG_GPU_DEBUG_GROUP("SkyboxPass")
        CG_GPU_TIME_FN(&renderingStats.skyboxTimer)

        Renderer::bindGraphicsPipeline(skyboxPipeline);
        Renderer::bindDescriptorSet(currentSceneEnvironment.environmentMapDescriptorSet, 0);
        Renderer::setPushConstants({skyboxPushConstants}, 1);
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
            collidersPushConstants->setData(&pc, sizeof(CollidersPushConstants));
            Renderer::setPushConstants({collidersPushConstants}, 1);
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
            collidersPushConstants->setData(&pc, sizeof(CollidersPushConstants));
            Renderer::setPushConstants({collidersPushConstants}, 1);
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
            collidersPushConstants->setData(&pc, sizeof(CollidersPushConstants));
            Renderer::setPushConstants({collidersPushConstants}, 1);
            Renderer::executeDrawCommand(command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount);
        }
    }

    void SceneRenderer::debugLinesPass() {
        Renderer::bindGraphicsPipeline(debugLinesPipeline);
        Renderer::bindDescriptorSet(debugLinesDescriptorSet, 0);
        Renderer::drawArrays(debugLinesVAO, debugLinesDrawInfoQueue.size() * 2);
    }

    void SceneRenderer::bloomPass() {
        CG_GPU_DEBUG_GROUP("BloomPass")
        CG_GPU_TIME_FN(&renderingStats.bloomTimer)

        BloomDownsamplePushConstants pc{};
        pc.useThreshold = true;
        bloomDownsamplePushConstants->setData(&pc, sizeof(BloomDownsamplePushConstants));

        Renderer::beginRenderPass(bloomDownSamplePass, bloomDownsampleFramebuffers[0]);
        Renderer::bindGraphicsPipeline(bloomDownsamplePipeline);
        Renderer::setPushConstants({bloomDownsamplePushConstants}, 1);
        Renderer::bindDescriptorSet(bloomDescriptorSets[0], 0);
        Renderer::renderUnitQuad();
        Renderer::endRenderPass();

        pc.useThreshold = false;
        bloomDownsamplePushConstants->setData(&pc, sizeof(BloomDownsamplePushConstants));

        for (uint32_t i = 0; i < bloomDownsampleFramebuffers.size() - 1; ++i) {
            Renderer::beginRenderPass(bloomDownSamplePass, bloomDownsampleFramebuffers[i + 1]);
            Renderer::bindGraphicsPipeline(bloomDownsamplePipeline);
            Renderer::setPushConstants({bloomDownsamplePushConstants}, 1);
            Renderer::bindDescriptorSet(bloomDescriptorSets[i + 1], 0);
            Renderer::renderUnitQuad();
            Renderer::endRenderPass();
        }

        for (uint32_t i = bloomUpsampleFramebuffers.size() - 1; i > 0; i--) {
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

    void SceneRenderer::uiPass() {
        CG_GPU_DEBUG_GROUP("UiPass")
        CG_GPU_TIME_FN(&renderingStats.uiTimer)

        uint32_t zIndex = 0;
        size_t circleOffset = 0;
        size_t rectOffset = 0;
        size_t textOffset = 0;

        for (const auto& [_, drawInfo]: uiDrawInfoQueue) {
            if (drawInfo.circleIndexCount > 0) {
                Renderer::bindGraphicsPipeline(uiCirclePipeline);
                Renderer::bindDescriptorSet(uiDescriptorSets[zIndex], 0);
                Renderer::executeDrawCommand(uiCircleVAO, drawInfo.circleIndexCount, 0, circleOffset);

                circleOffset += drawInfo.circleVertices.size();
            }
            if (drawInfo.rectIndexCount > 0) {
                Renderer::bindGraphicsPipeline(uiRectPipeline);
                Renderer::bindDescriptorSet(uiDescriptorSets[zIndex], 0);
                Renderer::executeDrawCommand(uiRectVAO, drawInfo.rectIndexCount, 0, rectOffset);

                rectOffset += drawInfo.rectVertices.size();
            }

            if (drawInfo.textIndexCount > 0) {
                Renderer::bindGraphicsPipeline(uiTextPipeline);
                Renderer::bindDescriptorSet(uiTextDescriptorSets[zIndex], 0);
                Renderer::executeDrawCommand(uiTextVAO, drawInfo.textIndexCount, 0, textOffset);

                textOffset += drawInfo.textVertices.size();
            }

            zIndex++;
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

        hbaoData.isOrtho = camera.getProjectionType() == CameraProjectionType::Orthographic;
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

    float SceneRenderer::findDrawInfoTextureIndex(UiDrawInfo& drawInfo, const Texture2D* texture) const {
        float textureIndex = -1;
        if (texture != nullptr) {
            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                if (drawInfo.textureSlots[i] == texture) {
                    textureIndex = static_cast<float>(i);
                    break;
                }
            }
            if (textureIndex < 0.0f) {
                textureIndex = static_cast<float>(drawInfo.filledTextureSlots);
                drawInfo.textureSlots[drawInfo.filledTextureSlots] = texture;
                drawInfo.filledTextureSlots++;
            }
        }
        return textureIndex;
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

    void SceneRenderer::buildUiVertexBuffers() {
        size_t circleOffset = 0;
        size_t rectOffset = 0;
        size_t textOffset = 0;
        uint32_t zIndex = 0;

        for (const auto& [_, drawInfo]: uiDrawInfoQueue) {
            if (drawInfo.circleIndexCount > 0) {
                size_t size = drawInfo.circleVertices.size() * sizeof(UiCircleVertex);
                uiCircleVAO->getVertexBuffer(0)->setSubData(circleOffset, drawInfo.circleVertices.data(), size);
                circleOffset += size;
            }
            if (drawInfo.rectIndexCount > 0) {
                size_t size = drawInfo.rectVertices.size() * sizeof(UiRectVertex);
                uiRectVAO->getVertexBuffer(0)->setSubData(rectOffset, drawInfo.rectVertices.data(), size);
                rectOffset += size;
            }
            if (drawInfo.textIndexCount > 0) {
                size_t size = drawInfo.textVertices.size() * sizeof(UiTextVertex);
                uiTextVAO->getVertexBuffer(0)->setSubData(textOffset, drawInfo.textVertices.data(), size);
                textOffset += size;
            }

            DescriptorSetSpecification uiDescriptorSetSpec{};
            uiDescriptorSetSpec.texture2DBindings.resize(16);
            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                uiDescriptorSetSpec.texture2DBindings[i].texture = drawInfo.textureSlots[i];
                uiDescriptorSetSpec.texture2DBindings[i].bindingPoint = i;
            }
            for (uint32_t i = drawInfo.filledTextureSlots; i < uiDescriptorSetSpec.texture2DBindings.size(); i++) {
                uiDescriptorSetSpec.texture2DBindings[i].texture = Renderer::getWhiteTexture();
                uiDescriptorSetSpec.texture2DBindings[i].bindingPoint = i;
            }
            uiDescriptorSets[zIndex]->reconfigure(uiDescriptorSetSpec);

            DescriptorSetSpecification uiTextDescriptorSetSpec{};
            uiTextDescriptorSetSpec.texture2DBindings.resize(4);
            for (uint32_t i = 0; i < drawInfo.filledFontAtlases; i++) {
                uiTextDescriptorSetSpec.texture2DBindings[i].texture = drawInfo.fontAtlases[i];
                uiTextDescriptorSetSpec.texture2DBindings[i].bindingPoint = i;
            }
            for (uint32_t i = drawInfo.filledFontAtlases; i < uiTextDescriptorSetSpec.texture2DBindings.size(); i++) {
                uiTextDescriptorSetSpec.texture2DBindings[i].texture = Renderer::getWhiteTexture();
                uiTextDescriptorSetSpec.texture2DBindings[i].bindingPoint = i;
            }
            uiTextDescriptorSets[zIndex]->reconfigure(uiTextDescriptorSetSpec);

            zIndex++;
        }
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
