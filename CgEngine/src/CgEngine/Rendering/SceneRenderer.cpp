#include "SceneRenderer.h"
#include "Asserts.h"
#include "Application.h"
#include "OpeGLTimer.h"

namespace CgEngine {
    SceneRenderer::SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight), invViewportWidth(1.0f / static_cast<float>(viewportWidth)), invViewportHeight(1.0f / static_cast<float>(viewportHeight)) {
        {
            ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

            dirShadowMaps = Texture2DArray(TextureFormat::Depth, applicationOptions.shadowMapResolution, applicationOptions.shadowMapResolution, TextureWrap::ClampBorder, 4, MipMapFiltering::Bilinear);
            dirShadowMaps.setClampBorderColor({1.0f, 1.0f, 1.0f, 1.0f});

            FramebufferSpecification shadowMapFramebufferSpec;
            shadowMapFramebufferSpec.height = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.width = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            shadowMapFramebufferSpec.hasDepthStencilAttachment = false;
            shadowMapFramebufferSpec.hasDepthAttachment = false;
            shadowMapFramebufferSpec.useExistingDepthAttachment = true;
            shadowMapFramebufferSpec.existingDepthAttachment = dirShadowMaps.getRendererId();

            auto* framebuffer = new Framebuffer(shadowMapFramebufferSpec);

            RenderPassSpecification shadowMapRenderPassSpec;
            shadowMapRenderPassSpec.shader = Shader("dirShadowMap");
            shadowMapRenderPassSpec.framebuffer = framebuffer;
            shadowMapRenderPassSpec.clearColorBuffer = false;
            shadowMapRenderPassSpec.clearDepthBuffer = true;
            shadowMapRenderPassSpec.clearStencilBuffer = false;
            shadowMapRenderPassSpec.frontfaceCulling = false;
            shadowMapRenderPassSpec.backfaceCulling = true;

            shadowMapRenderPass = RenderPass(std::move(shadowMapRenderPassSpec));
            shaderMap.dirShadowMapShader = &shadowMapRenderPass.getSpecification().shader;
        }
        {
            FramebufferSpecification preDepthFramebufferSpec;
            preDepthFramebufferSpec.height = viewportHeight;
            preDepthFramebufferSpec.width = viewportWidth;
            preDepthFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            preDepthFramebufferSpec.hasDepthStencilAttachment = false;
            preDepthFramebufferSpec.colorAttachments = {FramebufferFormat::RGB16F};
            preDepthFramebufferSpec.hasDepthAttachment = true;

            auto* framebuffer = new Framebuffer(preDepthFramebufferSpec);

            RenderPassSpecification preDepthRenderPassSpec;
            preDepthRenderPassSpec.shader = Shader("preDepth");
            preDepthRenderPassSpec.framebuffer = framebuffer;
            preDepthRenderPassSpec.clearColorBuffer = true;
            preDepthRenderPassSpec.clearDepthBuffer = true;
            preDepthRenderPassSpec.depthCompareOperator = DepthCompareOperator::Less;

            preDepthRenderPass = RenderPass(std::move(preDepthRenderPassSpec));
            shaderMap.preDepthShader = &preDepthRenderPass.getSpecification().shader;
        }
        {
            glm::uvec2 quarterSize = (glm::uvec2(viewportWidth, viewportHeight) + 3u) / 4u;

            hbaoDeinterleavingDepthTexture = Texture2DArray(TextureFormat::RedFloat32, quarterSize.x, quarterSize.y, TextureWrap::Clamp, 16, MipMapFiltering::Nearest);

            for (uint32_t i = 0; i < hbaoDeinterleavingDepthTextureViews.size(); i++) {
                hbaoDeinterleavingDepthTextureViews[i] = Texture2DView(
                        hbaoDeinterleavingDepthTexture.getRendererId(),
                        false,
                        hbaoDeinterleavingDepthTexture.getFormat(),
                        TextureWrap::Clamp,
                        0, 1, i, 1,
                        MipMapFiltering::Nearest
                );
            }

            FramebufferSpecification hbaoDeinterleavingFramebufferSpec0;
            hbaoDeinterleavingFramebufferSpec0.width = quarterSize.x;
            hbaoDeinterleavingFramebufferSpec0.height = quarterSize.y;
            hbaoDeinterleavingFramebufferSpec0.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            hbaoDeinterleavingFramebufferSpec0.hasDepthStencilAttachment = false;
            hbaoDeinterleavingFramebufferSpec0.hasDepthAttachment = false;
            hbaoDeinterleavingFramebufferSpec0.useExistingColorAttachment = true;
            hbaoDeinterleavingFramebufferSpec0.existingColorAttachments = {
                    hbaoDeinterleavingDepthTextureViews[0].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[1].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[2].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[3].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[4].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[5].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[6].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[7].getRendererId()
            };
            hbaoDeinterleavingFramebuffers[0] = new Framebuffer(hbaoDeinterleavingFramebufferSpec0);

            FramebufferSpecification hbaoDeinterleavingFramebufferSpec1;
            hbaoDeinterleavingFramebufferSpec1.width = quarterSize.x;
            hbaoDeinterleavingFramebufferSpec1.height = quarterSize.y;
            hbaoDeinterleavingFramebufferSpec1.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            hbaoDeinterleavingFramebufferSpec1.hasDepthStencilAttachment = false;
            hbaoDeinterleavingFramebufferSpec1.hasDepthAttachment = false;
            hbaoDeinterleavingFramebufferSpec1.useExistingColorAttachment = true;
            hbaoDeinterleavingFramebufferSpec1.existingColorAttachments = {
                    hbaoDeinterleavingDepthTextureViews[8].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[9].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[10].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[11].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[12].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[13].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[14].getRendererId(),
                    hbaoDeinterleavingDepthTextureViews[15].getRendererId()
            };
            hbaoDeinterleavingFramebuffers[1] = new Framebuffer(hbaoDeinterleavingFramebufferSpec1);

            RenderPassSpecification hbaoDeinterleavingRenderPassSpec;
            hbaoDeinterleavingRenderPassSpec.shader = Shader("hbaoDeinterleaving");
            hbaoDeinterleavingRenderPassSpec.framebuffer = hbaoDeinterleavingFramebuffers[0];
            hbaoDeinterleavingRenderPassSpec.usingExistingFramebuffer = true;
            hbaoDeinterleavingRenderPassSpec.clearDepthBuffer = false;
            hbaoDeinterleavingRenderPassSpec.clearColorBuffer = true;
            hbaoDeinterleavingRenderPassSpec.depthWrite = false;
            hbaoDeinterleavingRenderPassSpec.depthTest = false;

            hbaoDeinterleavingRenderPass = RenderPass(std::move(hbaoDeinterleavingRenderPassSpec));
            shaderMap.hbaoDeinterleavingShader = &hbaoDeinterleavingRenderPass.getSpecification().shader;

            hbaoShader = ComputeShader("hbao");
            shaderMap.hbaoShader = &hbaoShader;

            for (int i = 0; i < 16; i++) {
                hbaoData.float2Offsets[i] = glm::vec4((float)(i % 4) + 0.5f, (float)(i / 4.0f) + 0.5f, 0.0f, 1.f);
            }
            std::memcpy(hbaoData.jitters, generateHBAOJitterNoise().data(), sizeof(glm::vec4) * 16);

            hbaoResultTexture = Texture2DArray(TextureFormat::RedGreenFloat16, quarterSize.x, quarterSize.y, TextureWrap::Clamp, 16, MipMapFiltering::Nearest);

            FramebufferSpecification hbaoReinterleavingFramebufferSpec;
            hbaoReinterleavingFramebufferSpec.width = viewportWidth;
            hbaoReinterleavingFramebufferSpec.height = viewportHeight;
            hbaoReinterleavingFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            hbaoReinterleavingFramebufferSpec.hasDepthStencilAttachment = false;
            hbaoReinterleavingFramebufferSpec.hasDepthAttachment = false;
            hbaoReinterleavingFramebufferSpec.useExistingColorAttachment = false;
            hbaoReinterleavingFramebufferSpec.colorAttachments = {FramebufferFormat::RG16F};

            auto* hbaoReinterleavingFramebuffer = new Framebuffer(hbaoReinterleavingFramebufferSpec);

            RenderPassSpecification hbaoReinterleavingRenderPassSpec;
            hbaoReinterleavingRenderPassSpec.shader = Shader("hbaoReinterleaving");
            hbaoReinterleavingRenderPassSpec.framebuffer = hbaoReinterleavingFramebuffer;
            hbaoReinterleavingRenderPassSpec.clearColorBuffer = true;
            hbaoReinterleavingRenderPassSpec.clearDepthBuffer = false;
            hbaoReinterleavingRenderPassSpec.clearStencilBuffer = false;
            hbaoReinterleavingRenderPassSpec.frontfaceCulling = false;
            hbaoReinterleavingRenderPassSpec.backfaceCulling = false;
            hbaoReinterleavingRenderPassSpec.depthTest = false;
            hbaoReinterleavingRenderPassSpec.depthWrite = false;

            hbaoReinterleavingRenderPass = RenderPass(std::move(hbaoReinterleavingRenderPassSpec));
            shaderMap.hbaoReinterleavingShader = &hbaoReinterleavingRenderPass.getSpecification().shader;

            FramebufferSpecification hbaoBlurFramebufferSpec;
            hbaoBlurFramebufferSpec.width = viewportWidth;
            hbaoBlurFramebufferSpec.height = viewportHeight;
            hbaoBlurFramebufferSpec.clearColor = {1.0f, 1.0f, 1.0f, 0.0f};
            hbaoBlurFramebufferSpec.hasDepthStencilAttachment = false;
            hbaoBlurFramebufferSpec.hasDepthAttachment = false;
            hbaoBlurFramebufferSpec.useExistingColorAttachment = false;
            hbaoBlurFramebufferSpec.colorAttachments = {FramebufferFormat::RG16F};

            auto* hbaoBlurFramebuffer = new Framebuffer(hbaoBlurFramebufferSpec);

            RenderPassSpecification hbaoBlurRenderPassSpec;
            hbaoBlurRenderPassSpec.shader = Shader("hbaoBlur");
            hbaoBlurRenderPassSpec.framebuffer = hbaoBlurFramebuffer;
            hbaoBlurRenderPassSpec.clearColorBuffer = true;
            hbaoBlurRenderPassSpec.clearDepthBuffer = false;
            hbaoBlurRenderPassSpec.clearStencilBuffer = false;
            hbaoBlurRenderPassSpec.frontfaceCulling = false;
            hbaoBlurRenderPassSpec.backfaceCulling = false;
            hbaoBlurRenderPassSpec.depthTest = false;
            hbaoBlurRenderPassSpec.depthWrite = false;

            hbaoBlurRenderPass = RenderPass(std::move(hbaoBlurRenderPassSpec));
            shaderMap.hbaoBlurShader = &hbaoBlurRenderPass.getSpecification().shader;
        }
        {
            FramebufferSpecification geoFramebufferSpec;
            geoFramebufferSpec.height = viewportHeight;
            geoFramebufferSpec.width = viewportWidth;
            geoFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            geoFramebufferSpec.colorAttachments = {FramebufferFormat::RGBA16F};
            geoFramebufferSpec.hasDepthStencilAttachment = false;
            geoFramebufferSpec.useExistingDepthAttachment = true;
            geoFramebufferSpec.existingDepthAttachment = preDepthRenderPass.getSpecification().framebuffer->getDepthAttachmentRendererId();
            geoFramebufferSpec.samples = 1;

            auto* framebuffer = new Framebuffer(geoFramebufferSpec);

            RenderPassSpecification geoRenderPassSpec;
            geoRenderPassSpec.shader = Shader("pbr");
            geoRenderPassSpec.framebuffer = framebuffer;
            geoRenderPassSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            geoRenderPassSpec.clearDepthBuffer = false;
            geoRenderPassSpec.depthWrite = false;

            geometryRenderPass = RenderPass(std::move(geoRenderPassSpec));
            shaderMap.geometryShader = &geometryRenderPass.getSpecification().shader;
        }
        {
            RenderPassSpecification customShaderRenderPassSpec;
            customShaderRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            customShaderRenderPassSpec.usingExistingFramebuffer = true;
            customShaderRenderPassSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            customShaderRenderPassSpec.depthWrite = true;
            customShaderRenderPassSpec.depthTest = true;
            customShaderRenderPassSpec.clearColorBuffer = false;
            customShaderRenderPassSpec.clearDepthBuffer = false;
            customShaderRenderPassSpec.clearStencilBuffer = false;

            customShaderRenderPass = RenderPass(std::move(customShaderRenderPassSpec));
        }
        {
            RenderPassSpecification skyboxRenderPassSpec;
            skyboxRenderPassSpec.shader = Shader("skybox");
            skyboxRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            skyboxRenderPassSpec.usingExistingFramebuffer = true;
            skyboxRenderPassSpec.depthCompareOperator = DepthCompareOperator::LessOrEqual;
            skyboxRenderPassSpec.clearColorBuffer = false;
            skyboxRenderPassSpec.clearDepthBuffer = false;
            skyboxRenderPassSpec.clearStencilBuffer = false;

            skyboxRenderPass = RenderPass(std::move(skyboxRenderPassSpec));
            shaderMap.skyboxShader = &skyboxRenderPass.getSpecification().shader;
        }
        {
            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;
            for (auto& bloomTexture: bloomTextures) {
                bloomTexture = Texture2D(TextureFormat::Float32, static_cast<uint32_t>(bloomWidth), static_cast<uint32_t>(bloomHeight), TextureWrap::Clamp, MipMapFiltering::Bilinear);
                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }

            FramebufferSpecification bloomFramebufferSpec;
            bloomFramebufferSpec.width = viewportWidth / 2;
            bloomFramebufferSpec.height = viewportHeight / 2;
            bloomFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            bloomFramebufferSpec.hasDepthStencilAttachment = false;
            bloomFramebufferSpec.useExistingColorAttachment = true;
            bloomFramebufferSpec.existingColorAttachments = { bloomTextures[0].getRendererId() };

            auto* framebuffer = new Framebuffer(bloomFramebufferSpec);

            RenderPassSpecification bloomDownSamplePassSpec;
            bloomDownSamplePassSpec.shader = Shader("bloomDownSample");
            bloomDownSamplePassSpec.clearColorBuffer = true;
            bloomDownSamplePassSpec.clearDepthBuffer = false;
            bloomDownSamplePassSpec.depthTest = false;
            bloomDownSamplePassSpec.depthWrite = false;
            bloomDownSamplePassSpec.framebuffer = framebuffer;

            bloomDownSamplePass = RenderPass(std::move(bloomDownSamplePassSpec));
            shaderMap.bloomDownSampleShader = &bloomDownSamplePass.getSpecification().shader;

            RenderPassSpecification bloomUpSamplePassSpec;
            bloomUpSamplePassSpec.shader = Shader("bloomUpSample");
            bloomUpSamplePassSpec.clearColorBuffer = false;
            bloomUpSamplePassSpec.clearDepthBuffer = false;
            bloomUpSamplePassSpec.depthTest = false;
            bloomUpSamplePassSpec.depthWrite = false;
            bloomUpSamplePassSpec.useBlending = true;
            bloomUpSamplePassSpec.blendingEquation = BlendingEquation::Add;
            bloomUpSamplePassSpec.srcBlendingFunction = BlendingFunction::One;
            bloomUpSamplePassSpec.destBlendingFunction = BlendingFunction::One;
            bloomUpSamplePassSpec.framebuffer = framebuffer;
            bloomUpSamplePassSpec.usingExistingFramebuffer = true;

            bloomUpSamplePass = RenderPass(std::move(bloomUpSamplePassSpec));
            shaderMap.bloomUpSampleShader = &bloomUpSamplePass.getSpecification().shader;
        }
        {
            RenderPassSpecification physicsCollidersRenderPassSpec;
            physicsCollidersRenderPassSpec.shader = Shader("colliders");
            physicsCollidersRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            physicsCollidersRenderPassSpec.usingExistingFramebuffer = true;
            physicsCollidersRenderPassSpec.depthTest = false;
            physicsCollidersRenderPassSpec.depthWrite = false;
            physicsCollidersRenderPassSpec.clearColorBuffer = false;
            physicsCollidersRenderPassSpec.clearDepthBuffer = false;
            physicsCollidersRenderPassSpec.clearStencilBuffer = false;
            physicsCollidersRenderPassSpec.wireframe = true;

            physicsCollidersRenderPass = RenderPass(std::move(physicsCollidersRenderPassSpec));
            shaderMap.physicsCollidersShader = &physicsCollidersRenderPass.getSpecification().shader;

            physicsCollidersMaterial.set("u_Color", {0.0f, 1.0f, 0.0f});
        }
        {
            RenderPassSpecification boundingBoxRenderPassSpec;
            boundingBoxRenderPassSpec.shader = Shader("colliders");
            boundingBoxRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            boundingBoxRenderPassSpec.usingExistingFramebuffer = true;
            boundingBoxRenderPassSpec.depthTest = true;
            boundingBoxRenderPassSpec.depthWrite = false;
            boundingBoxRenderPassSpec.clearColorBuffer = false;
            boundingBoxRenderPassSpec.clearDepthBuffer = false;
            boundingBoxRenderPassSpec.clearStencilBuffer = false;
            boundingBoxRenderPassSpec.wireframe = true;
            boundingBoxRenderPassSpec.backfaceCulling = false;

            boundingBoxRenderPass = RenderPass(std::move(boundingBoxRenderPassSpec));
            shaderMap.boundingBoxShader = &boundingBoxRenderPass.getSpecification().shader;

            boundingBoxMaterial.set("u_Color", {1.0f, 1.0f, 0.0f});
        }
        {
            RenderPassSpecification mormalsDebugRenderPassSpec;
            mormalsDebugRenderPassSpec.shader = Shader("normalsVisualize");
            mormalsDebugRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            mormalsDebugRenderPassSpec.usingExistingFramebuffer = true;
            mormalsDebugRenderPassSpec.depthTest = true;
            mormalsDebugRenderPassSpec.depthWrite = false;
            mormalsDebugRenderPassSpec.clearColorBuffer = false;
            mormalsDebugRenderPassSpec.clearDepthBuffer = false;
            mormalsDebugRenderPassSpec.clearStencilBuffer = false;

            normalsDebugRenderPass = RenderPass(std::move(mormalsDebugRenderPassSpec));
            shaderMap.normalsDebugShader = &normalsDebugRenderPass.getSpecification().shader;

            normalsDebugMaterial.set("u_Color", {1.0f, 0.0f, 0.0f});
        }
        {
            RenderPassSpecification debugLinesRenderPassSpec;
            debugLinesRenderPassSpec.shader = Shader("lines");
            debugLinesRenderPassSpec.framebuffer = geometryRenderPass.getSpecification().framebuffer;
            debugLinesRenderPassSpec.usingExistingFramebuffer = true;
            debugLinesRenderPassSpec.depthTest = true;
            debugLinesRenderPassSpec.depthWrite = false;
            debugLinesRenderPassSpec.clearColorBuffer = false;
            debugLinesRenderPassSpec.clearDepthBuffer = false;
            debugLinesRenderPassSpec.clearStencilBuffer = false;

            debugLinesRenderPass = RenderPass(std::move(debugLinesRenderPassSpec));
            shaderMap.debugLinesShader = &debugLinesRenderPass.getSpecification().shader;
        }
        {
            FramebufferSpecification screenFramebufferSpec;
            screenFramebufferSpec.height = viewportHeight;
            screenFramebufferSpec.width = viewportWidth;
            screenFramebufferSpec.clearColor = {0.0f, 1.0f, 1.0f, 1.0f};
            screenFramebufferSpec.hasDepthStencilAttachment = false;
            screenFramebufferSpec.samples = 1;
            screenFramebufferSpec.screenTarget = true;

            auto* framebuffer = new Framebuffer(screenFramebufferSpec);
            RenderPassSpecification screenRenderPassSpec;
            screenRenderPassSpec.shader = Shader("screen");
            screenRenderPassSpec.framebuffer = framebuffer;
            screenRenderPassSpec.depthTest = false;

            screenRenderPass = RenderPass(std::move(screenRenderPassSpec));
            shaderMap.screenShader = &screenRenderPass.getSpecification().shader;

            screenMaterial.setTexture("u_FinalImage", geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
            screenMaterial.setTexture("u_BloomTexture", bloomTextures[0].getRendererId(), 1);
        }
        {
            RenderPassSpecification uiCircleRenderPassSpec;
            uiCircleRenderPassSpec.shader = Shader("uiCircle");
            uiCircleRenderPassSpec.clearColorBuffer = false;
            uiCircleRenderPassSpec.clearDepthBuffer = false;
            uiCircleRenderPassSpec.depthTest = false;
            uiCircleRenderPassSpec.depthWrite = false;
            uiCircleRenderPassSpec.useBlending = true;
            uiCircleRenderPassSpec.blendingEquation = BlendingEquation::Add;
            uiCircleRenderPassSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiCircleRenderPassSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiCircleRenderPassSpec.framebuffer = screenRenderPass.getSpecification().framebuffer;
            uiCircleRenderPassSpec.usingExistingFramebuffer = true;

            uiCirclePass = RenderPass(std::move(uiCircleRenderPassSpec));
            shaderMap.uiCircleShader = &uiCirclePass.getSpecification().shader;

            RenderPassSpecification uiRectRenderPassSpec;
            uiRectRenderPassSpec.shader = Shader("uiRect");
            uiRectRenderPassSpec.clearColorBuffer = false;
            uiRectRenderPassSpec.clearDepthBuffer = false;
            uiRectRenderPassSpec.depthTest = false;
            uiRectRenderPassSpec.depthWrite = false;
            uiRectRenderPassSpec.useBlending = true;
            uiRectRenderPassSpec.blendingEquation = BlendingEquation::Add;
            uiRectRenderPassSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiRectRenderPassSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiRectRenderPassSpec.framebuffer = screenRenderPass.getSpecification().framebuffer;
            uiRectRenderPassSpec.usingExistingFramebuffer = true;

            uiRectPass = RenderPass(std::move(uiRectRenderPassSpec));
            shaderMap.uiRectShader = &uiRectPass.getSpecification().shader;

            RenderPassSpecification uiTextRenderPassSpec;
            uiTextRenderPassSpec.shader = Shader("uiText");
            uiTextRenderPassSpec.clearColorBuffer = false;
            uiTextRenderPassSpec.clearDepthBuffer = false;
            uiTextRenderPassSpec.depthTest = false;
            uiTextRenderPassSpec.depthWrite = false;
            uiTextRenderPassSpec.useBlending = true;
            uiTextRenderPassSpec.blendingEquation = BlendingEquation::Add;
            uiTextRenderPassSpec.srcBlendingFunction = BlendingFunction::SrcAlpha;
            uiTextRenderPassSpec.destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
            uiTextRenderPassSpec.framebuffer = screenRenderPass.getSpecification().framebuffer;
            uiTextRenderPassSpec.usingExistingFramebuffer = true;

            uiTextPass = RenderPass(std::move(uiTextRenderPassSpec));
            shaderMap.uiTextShader = &uiTextPass.getSpecification().shader;


            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

        for (uint32_t i = 0; i < Renderer::maxTextureSlots; i++) {
            Renderer::getWhiteTexture().bind(i);
        }

        ubCameraData = new UniformBuffer<UBCameraData>("CameraData", 0, geometryRenderPass.getSpecification().shader);
        ubLightData = new UniformBuffer<UBLightData>("LightData", 1, geometryRenderPass.getSpecification().shader);
        ubDirShadowData = new UniformBuffer<UBDirShadowData>("DirShadowData", 2, shadowMapRenderPass.getSpecification().shader);
        ubScreenData = new UniformBuffer<UBScreenData>("ScreenData", 3, hbaoDeinterleavingRenderPass.getSpecification().shader);
        ubHBAOData = new UniformBuffer<UBHBAOData>("HBAOData", 4, hbaoShader);

        boneTransformsBuffer = new ShaderStorageBuffer();
        boneTransformsBuffer->setData(nullptr, maxBones * maxAnimatedComponents * sizeof(glm::mat4));

        skinningShader = ComputeShader("skinning");
        shaderMap.skinningShader = &skinningShader;
    }

    SceneRenderer::~SceneRenderer() {
        delete hbaoDeinterleavingFramebuffers[0];
        delete hbaoDeinterleavingFramebuffers[1];

        delete ubCameraData;
        delete ubLightData;
        delete ubDirShadowData;
        delete ubScreenData;

        delete boneTransformsBuffer;
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
            ubScreenData->setData(screenData);

            preDepthRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);
            geometryRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);
            geometryRenderPass.getSpecification().framebuffer->setDepthAttachment(preDepthRenderPass.getSpecification().framebuffer->getDepthAttachmentRendererId(), 0, viewportWidth, viewportHeight);
            screenRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);

            glm::uvec2 quarterSize = (glm::uvec2(viewportWidth, viewportHeight) + 3u) / 4u;

            hbaoDeinterleavingDepthTexture = Texture2DArray(TextureFormat::RedFloat32, quarterSize.x, quarterSize.y, TextureWrap::Clamp, 16, MipMapFiltering::Nearest);

            for (int i = 0; i < hbaoDeinterleavingDepthTextureViews.size(); i++) {
                hbaoDeinterleavingDepthTextureViews[i] = Texture2DView(
                        hbaoDeinterleavingDepthTexture.getRendererId(),
                        false,
                        hbaoDeinterleavingDepthTexture.getFormat(),
                        TextureWrap::Clamp,
                        0, 1, i, 1,
                        MipMapFiltering::Nearest
                );
            }

            hbaoDeinterleavingFramebuffers[0]->resize(quarterSize.x, quarterSize.y, false);
            hbaoDeinterleavingFramebuffers[0]->setColorAttachments({
                hbaoDeinterleavingDepthTextureViews[0].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[1].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[2].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[3].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[4].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[5].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[6].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[7].getRendererId()
            }, 0, quarterSize.x, quarterSize.y);

            hbaoDeinterleavingFramebuffers[1]->resize(quarterSize.x, quarterSize.y, false);
            hbaoDeinterleavingFramebuffers[1]->setColorAttachments({
                hbaoDeinterleavingDepthTextureViews[8].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[9].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[10].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[11].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[12].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[13].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[14].getRendererId(),
                hbaoDeinterleavingDepthTextureViews[15].getRendererId()
            }, 0, quarterSize.x, quarterSize.y);

            constexpr uint32_t HBAO_WORK_GROUP_SIZE = 16u;
            glm::uvec2 quarterSizeWorkGroups = quarterSize + (HBAO_WORK_GROUP_SIZE - quarterSize % HBAO_WORK_GROUP_SIZE);
            hbaoWorkGroupSize.x = quarterSizeWorkGroups.x / 16u;
            hbaoWorkGroupSize.y = quarterSizeWorkGroups.y / 16u;
            hbaoWorkGroupSize.z = 16u;

            hbaoResultTexture = Texture2DArray(TextureFormat::RedGreenFloat16, quarterSize.x, quarterSize.y, TextureWrap::Clamp, 16, MipMapFiltering::Nearest);

            hbaoReinterleavingRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);
            hbaoBlurRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);

            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;
            for (int i = 0; i < bloomTextures.size(); i++) {
                bloomTextures[i] = Texture2D(TextureFormat::Float32, static_cast<uint32_t>(bloomWidth), static_cast<uint32_t>(bloomHeight), TextureWrap::Clamp, MipMapFiltering::Bilinear);
                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }

            screenMaterial.setTexture("u_FinalImage", geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
            screenMaterial.setTexture("u_BloomTexture", bloomTextures[0].getRendererId(), 1);

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

        UBCameraData cameraData{};
        cameraData.projection = camera.getProjectionMatrix();
        cameraData.view = glm::inverse(cameraTransform);
        cameraData.viewProjection = cameraData.projection * cameraData.view;
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
        ubCameraData->setData(cameraData);

        cameraFrustum.updateCameraFrustum(camera, cameraTransform[3], -cameraTransform[2]);

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

        ubLightData->setData(lightData);

        skyboxMaterial.setTextureCube("u_Texture", *sceneEnvironment.prefilterMap, 0);
        skyboxMaterial.set("u_Intensity", sceneEnvironment.environmentIntensity);
        skyboxMaterial.set("u_Lod", sceneEnvironment.environmentLod);

        currentSceneEnvironment.environmentIntensity = sceneEnvironment.environmentIntensity;
        currentSceneEnvironment.irradianceMapId = sceneEnvironment.irradianceMap->getRendererId();
        currentSceneEnvironment.prefilterMapId = sceneEnvironment.prefilterMap->getRendererId();
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

        skinMeshes();
        shadowMapPass();
        preDepthPass();

        if (applicationOptions.enableHBAO) {
            hbaoDeinterleavingPass();
            hbaoComputePass();
            hbaoReinterleavingPass();
            hbaoBlurPass();
        } else {
            clearPass(hbaoBlurRenderPass);
        }

        geometryPass();
        customShaderPass();
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

        if (applicationOptions.enableBloom) {
            bloomPass();
        }
        screenPass();
        uiPass();

        skinningQueue.clear();

        drawCommandQueue.clear();
        meshTransforms.clear();

        customShaderDrawCommandQueue.clear();

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

    void SceneRenderer::submitMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* overrideMaterial, bool castShadows, bool enableCulling, const glm::mat4& transform) {
        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);


            bool isInCameraFrustum = !enableCulling || cameraFrustum.testAABoundingBoxInFrustum(meshNode.aaBoundingBox, transform * meshNode.transform);

            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);

                glm::mat4 finalTransform;

                if (isInCameraFrustum || castShadows) {
                    finalTransform = transform * meshNode.transform;
                }

                if (isInCameraFrustum) {
                    const Material* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
                    MeshKey mk = {mesh->getVAO()->getRendererId(), submeshIndex, material->getUuid().getUuid()};

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
                    MeshKey mk = {mesh->getVAO()->getRendererId(), submeshIndex, 0};

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
        CG_ASSERT(boneTransforms.size() <= maxBones, "Mesh contains to many bones")
        CG_ASSERT(skinningQueue.size() < maxAnimatedComponents, "Cannot render that many AnimatedMeshRendererComponents")

        uint32_t boneTransformOffset = skinningQueue.size() * maxBones * sizeof(glm::mat4);
        boneTransformsBuffer->setSubData(boneTransformOffset, boneTransforms.data(), boneTransforms.size() * sizeof(glm::mat4));

        SkinningInfo& skinningInfo = skinningQueue.emplace_back();
        skinningInfo.originalVertexBuffer = mesh->getVAO()->getVertexBuffers()[0];
        skinningInfo.skinnedVertexBuffer = skinnedVAO->getVertexBuffers()[0];
        skinningInfo.boneInfluencesBuffer = mesh->getBoneInfluencesBuffer();
        skinningInfo.numVertices = mesh->getVertices().size();

        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            const auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);

            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);
                const Material* material = overrideMaterial != nullptr ? overrideMaterial : mesh->getMaterial(submesh.materialIndex);
                MeshKey mk = {skinnedVAO->getRendererId(), submeshIndex, material->getUuid().getUuid()};

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

    void SceneRenderer::submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* material, bool enableCulling, AABoundingBox& boundingBox, const glm::mat4& transform, CustomShader* shader, uint32_t instanceCount, CustomShaderRendererComponentRenderPassOptions& renderPassOptions, std::pair<ShaderStorageBuffer*, ShaderStorageBuffer*> instanceBuffers) {
        if (enableCulling && !cameraFrustum.testAABoundingBoxInFrustum(boundingBox, transform)) {
            return;
        }

        auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNodeIndex: meshNodes) {
            auto& meshNode = mesh->getMeshNodes().at(meshNodeIndex);

            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);

                CustomShaderDrawCommand& drawCommand = customShaderDrawCommandQueue[shader].emplace_back();
                drawCommand.instanceCount = instanceCount;
                drawCommand.vao = mesh->getVAO();
                drawCommand.material = material;
                drawCommand.baseIndex = submesh.baseIndex;
                drawCommand.baseVertex = submesh.baseVertex;
                drawCommand.indexCount = submesh.indexCount;
                drawCommand.transform = transform * meshNode.transform;
                drawCommand.renderPassOptions = renderPassOptions;
                drawCommand.instanceBuffers = instanceBuffers;
            }
        }
    }

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
                    if (*drawInfo.fontAtlases[i] == *fontAtlas) {
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

            CG_ASSERT(drawInfo.circleIndexCount <= Renderer::maxUiIndices, "Cannot render that many UICircles")
            CG_ASSERT(drawInfo.rectIndexCount <= Renderer::maxUiIndices, "Cannot render that many UIRects")
            CG_ASSERT(drawInfo.textIndexCount <= Renderer::maxUiIndices, "Cannot render that many UIText")
            CG_ASSERT(drawInfo.filledTextureSlots < drawInfo.textureSlots.size(), "Cannot render that many different Textures on a single z-index")
            CG_ASSERT(drawInfo.filledFontAtlases < drawInfo.fontAtlases.size(), "Cannot render that many different Fonts on a single z-index")
        }
    }

    void SceneRenderer::submitPhysicsColliderMesh(MeshVertices* mesh, const glm::mat4& transform) {
        const auto& submeshes = mesh->getSubmeshes();

        for (const auto& meshNode: mesh->getMeshNodes()) {
            for (const auto& submeshIndex: meshNode.submeshIndices) {
                const Submesh& submesh = submeshes.at(submeshIndex);
                MeshKey mk = {mesh->getVAO()->getRendererId(), submeshIndex, physicsCollidersMaterial.getUuid().getUuid()};

                physicsCollidersMeshTransforms[mk].emplace_back(transform * meshNode.transform);

                DrawCommand& drawCommand = physicsCollidersDrawCommandQueue[mk];
                drawCommand.vao = mesh->getVAO();
                drawCommand.material = &physicsCollidersMaterial;
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

            MeshKey mk = {boundingBoxMesh->getVAO()->getRendererId(), 0, boundingBoxMaterial.getUuid().getUuid()};
            boundingBoxMeshTransforms[mk].emplace_back(glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), extents * 2.0f));

            DrawCommand& drawCommand = boundingBoxDrawCommandQueue[mk];
            drawCommand.vao = boundingBoxMesh->getVAO();
            drawCommand.material = &boundingBoxMaterial;
            drawCommand.baseIndex = boundingBoxSubmesh.baseIndex;
            drawCommand.baseVertex = boundingBoxSubmesh.baseVertex;
            drawCommand.indexCount = boundingBoxSubmesh.indexCount;
            drawCommand.instanceCount++;
        }
    }

    void SceneRenderer::submitBoundingBoxMesh(CgEngine::MeshVertices* boundingBoxMesh, CgEngine::AABoundingBox& boundingBox, const glm::mat4& transform) {
        auto [center, extents] = boundingBox.getTransformedAdjustedCenterAndExtents(transform);

        const auto& boundingBoxSubmesh = boundingBoxMesh->getSubmeshes().at(0);

        MeshKey mk = {boundingBoxMesh->getVAO()->getRendererId(), 0, boundingBoxMaterial.getUuid().getUuid()};
        boundingBoxMeshTransforms[mk].emplace_back(glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), extents * 2.0f));

        DrawCommand& drawCommand = boundingBoxDrawCommandQueue[mk];
        drawCommand.vao = boundingBoxMesh->getVAO();
        drawCommand.material = &boundingBoxMaterial;
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

    ShaderMap& SceneRenderer::getShaderMap() {
        return shaderMap;
    }

    void SceneRenderer::skinMeshes() {
        CG_GPU_TIME_FN(&renderingStats.skinMeshesTimer, false)

        skinningShader.bind();
        boneTransformsBuffer->bind(2);

        for (uint32_t i = 0; i < skinningQueue.size(); i++) {
            skinningQueue[i].originalVertexBuffer->bindAsSSBO(3);
            skinningQueue[i].skinnedVertexBuffer->bindAsSSBO(4);
            skinningQueue[i].boneInfluencesBuffer->bind(1);

            skinningShader.setInt("u_ComponentIndex", i);
            skinningShader.dispatch((skinningQueue[i].numVertices / 32) + 1, 1, 1);
            skinningShader.waitForMemoryBarrier({MemoryBarrierBit::All});
        }
    }

    void SceneRenderer::shadowMapPass() {
        CG_GPU_TIME_FN(&renderingStats.shadowMapTimer, false)

        if (!currentSceneEnvironment.dirLightCastShadows) {
            clearPass(shadowMapRenderPass);
            return;
        }

        Renderer::beginRenderPass(shadowMapRenderPass);

        for (const auto [mk, command]: shadowMapDrawCommandQueue) {
            const auto& transforms = shadowMapMeshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, emptyMaterial, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::preDepthPass() {
        CG_GPU_TIME_FN(&renderingStats.preDepthTimer, false)

        Renderer::beginRenderPass(preDepthRenderPass);

        for (const auto [mk, command]: drawCommandQueue) {
            const auto& transforms = meshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, emptyMaterial, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoDeinterleavingPass() {
        CG_GPU_TIME_FN(&renderingStats.hbaoDeinterleavingTimer, false)

        auto& deinterleavingShader = hbaoDeinterleavingRenderPass.getSpecification().shader;

        hbaoDeinterleavingRenderPass.getSpecification().framebuffer = hbaoDeinterleavingFramebuffers[0];
        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass);
        deinterleavingShader.setTexture(preDepthRenderPass.getSpecification().framebuffer->getDepthAttachmentRendererId(), 0);
        deinterleavingShader.setInt("u_UVOffsetIndex", 0);
        Renderer::renderUnitQuad(emptyMaterial);
        Renderer::endRenderPass();

        hbaoDeinterleavingRenderPass.getSpecification().framebuffer = hbaoDeinterleavingFramebuffers[1];
        Renderer::beginRenderPass(hbaoDeinterleavingRenderPass);
        deinterleavingShader.setInt("u_UVOffsetIndex", 1);
        Renderer::renderUnitQuad(emptyMaterial);
        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoComputePass() {
        CG_GPU_TIME_FN(&renderingStats.hbaoComputeTimer, false)

        hbaoShader.bind();

        hbaoShader.setTexture2D(hbaoDeinterleavingDepthTexture.getRendererId(), 0);
        hbaoShader.setTexture2D(preDepthRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 1);
        hbaoShader.setImageArray(hbaoResultTexture, 2, ShaderStorageAccess::WriteOnly);

        hbaoShader.dispatch(hbaoWorkGroupSize.x, hbaoWorkGroupSize.y, hbaoWorkGroupSize.z);
        hbaoShader.waitForMemoryBarrier({MemoryBarrierBit::TextureFetch, MemoryBarrierBit::ShaderImageAccess});
    }

    void SceneRenderer::hbaoReinterleavingPass() {
        CG_GPU_TIME_FN(&renderingStats.hbaoReinterleavingTimer, false)

        Renderer::beginRenderPass(hbaoReinterleavingRenderPass);
        hbaoReinterleavingRenderPass.getSpecification().shader.setTexture(hbaoResultTexture.getRendererId(), 0);
        Renderer::renderUnitQuad(emptyMaterial);
        Renderer::endRenderPass();
    }

    void SceneRenderer::hbaoBlurPass() {
        CG_GPU_TIME_FN(&renderingStats.hbaoBlurTimer, false)

        auto& shader = hbaoBlurRenderPass.getSpecification().shader;

        Renderer::beginRenderPass(hbaoBlurRenderPass);

        shader.setFloat("u_Sharpness", hbaoSharpness);

        shader.setTexture(hbaoReinterleavingRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
        shader.setVec2("u_InvResolutionDirection", glm::vec2(invViewportWidth, 0.0f));
        Renderer::renderUnitQuad(emptyMaterial);

        shader.setTexture(hbaoBlurRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
        shader.setVec2("u_InvResolutionDirection", glm::vec2(0.0f, invViewportHeight));
        Renderer::renderUnitQuad(emptyMaterial);

        Renderer::endRenderPass();
    }

    void SceneRenderer::geometryPass() {
        CG_GPU_TIME_FN(&renderingStats.geometryTimer, false)

        Renderer::beginRenderPass(geometryRenderPass);

        geometryRenderPass.getSpecification().shader.setTexture(currentSceneEnvironment.irradianceMapId, 5);
        geometryRenderPass.getSpecification().shader.setTexture(currentSceneEnvironment.prefilterMapId, 6);
        geometryRenderPass.getSpecification().shader.setTexture(Renderer::getBrdfLUTTexture().getRendererId(), 7);
        geometryRenderPass.getSpecification().shader.setFloat("u_EnvironmentIntensity", currentSceneEnvironment.environmentIntensity);
        geometryRenderPass.getSpecification().shader.setTexture(dirShadowMaps.getRendererId(), 8);
        geometryRenderPass.getSpecification().shader.setTexture(hbaoBlurRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 9);

        for (const auto [mk, command]: drawCommandQueue) {
            const auto& transforms = meshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, *command.material, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::customShaderPass() {
        CG_GPU_TIME_FN(&renderingStats.customShaderTimer, false)

        Renderer::beginRenderPass(customShaderRenderPass, true);

        // can be true at the start, because the values are already bound from the previous geometryPass
        bool lastCommandUseDirShadowMappingData = true;
        bool lastCommandUseEnvironmentMappingData = true;

        const Material* lastUsedMaterial = nullptr;

        for (const auto& [shader, commands]: customShaderDrawCommandQueue) {
            shader->bind();

            for (const auto& command: commands) {
                shader->setMat4("u_Transform", command.transform);

                if (command.instanceBuffers.first != nullptr) {
                    command.instanceBuffers.first->bind(5);
                }
                if (command.instanceBuffers.second != nullptr) {
                    command.instanceBuffers.second->bind(6);
                }

                if (command.renderPassOptions.useDirShadowMappingData && !lastCommandUseDirShadowMappingData) {
                    shader->setTexture(dirShadowMaps.getRendererId(), 8);
                }
                lastCommandUseDirShadowMappingData = command.renderPassOptions.useDirShadowMappingData;

                if (command.renderPassOptions.useEnvironmentMappingData && !lastCommandUseEnvironmentMappingData) {
                    shader->setTexture(currentSceneEnvironment.irradianceMapId, 5);
                    shader->setTexture(currentSceneEnvironment.prefilterMapId, 6);
                    shader->setTexture(Renderer::getBrdfLUTTexture().getRendererId(), 7);
                    shader->setFloat("u_EnvironmentIntensity", currentSceneEnvironment.environmentIntensity);
                    shader->setTexture(dirShadowMaps.getRendererId(), 8);
                }
                lastCommandUseEnvironmentMappingData = command.renderPassOptions.useEnvironmentMappingData;

                const Material* material = command.material != nullptr ? command.material : &emptyMaterial;

                Renderer::setFaceCulling(command.renderPassOptions.backfaceCulling, command.renderPassOptions.frontfaceCulling);
                Renderer::setBlending(command.renderPassOptions.useBlending, command.renderPassOptions.blendingEquation, command.renderPassOptions.srcBlendingFunction, command.renderPassOptions.destBlendingFunction);
                Renderer::setWireframe(command.renderPassOptions.wireframe);
                Renderer::setTesselationPatchSize(command.renderPassOptions.tesselationPatchSize);

                if (material != lastUsedMaterial) {
                    material->uploadToShader(*shader);
                }
                lastUsedMaterial = material;

                Renderer::executeCustomShaderDrawCommand(*command.vao, command.indexCount, command.baseIndex, command.baseVertex, command.instanceCount, command.renderPassOptions.tesselationPatchSize);
            }
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::skyboxPass() {
        CG_GPU_TIME_FN(&renderingStats.skyboxTimer, false)

        Renderer::beginRenderPass(skyboxRenderPass);
        Renderer::renderUnitCube(skyboxMaterial);
        Renderer::endRenderPass();
    }

    void SceneRenderer::physicsCollidersPass() {
        Renderer::beginRenderPass(physicsCollidersRenderPass);

        for (const auto [mk, command]: physicsCollidersDrawCommandQueue) {
            const auto& transforms = physicsCollidersMeshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, *command.material, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::boundingBoxPass() {
        Renderer::beginRenderPass(boundingBoxRenderPass);

        for (const auto [mk, command]: boundingBoxDrawCommandQueue) {
            const auto& transforms = boundingBoxMeshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, *command.material, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::normalsDebugPass() {
        Renderer::beginRenderPass(normalsDebugRenderPass);

        for (const auto [mk, command]: drawCommandQueue) {
            const auto& transforms = meshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, normalsDebugMaterial, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::debugLinesPass() {
        Renderer::beginRenderPass(debugLinesRenderPass);
        Renderer::renderLines(debugLinesDrawInfoQueue);
        Renderer::endRenderPass();
    }

    void SceneRenderer::bloomPass() {
        CG_GPU_TIME_FN(&renderingStats.bloomTimer, false)

        auto& downSampleShader = bloomDownSamplePass.getSpecification().shader;

        bloomDownSamplePass.getSpecification().framebuffer->setColorAttachments({bloomTextures[0].getRendererId()}, 0, viewportWidth / 2, viewportHeight / 2);
        Renderer::beginRenderPass(bloomDownSamplePass);
        downSampleShader.setTexture(geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
        downSampleShader.setBool("u_UseThreshold", true);
        Renderer::renderUnitQuad(emptyMaterial);
        Renderer::endRenderPass();

        downSampleShader.setBool("u_UseThreshold", false);
        for (uint32_t i = 0; i < bloomTextures.size() - 1; ++i) {
            bloomDownSamplePass.getSpecification().framebuffer->setColorAttachments({bloomTextures[i + 1].getRendererId()}, 0, bloomTextures[i + 1].getWidth(), bloomTextures[i + 1].getHeight());
            Renderer::beginRenderPass(bloomDownSamplePass);
            downSampleShader.setTexture(bloomTextures[i].getRendererId(), 0);
            Renderer::renderUnitQuad(emptyMaterial);
            Renderer::endRenderPass();
        }

        auto& upSampleShader = bloomUpSamplePass.getSpecification().shader;

        for (uint32_t i = bloomTextures.size() - 1; i > 0; i--) {
            bloomUpSamplePass.getSpecification().framebuffer->setColorAttachments({bloomTextures[i - 1].getRendererId()}, 0, bloomTextures[i - 1].getWidth(), bloomTextures[i - 1].getHeight());
            Renderer::beginRenderPass(bloomUpSamplePass);
            upSampleShader.setTexture(bloomTextures[i].getRendererId(), 0);
            Renderer::renderUnitQuad(emptyMaterial);
            Renderer::endRenderPass();
        }
    }

    void SceneRenderer::screenPass() {
        CG_GPU_TIME_FN(&renderingStats.screenTimer, false)

        Renderer::beginRenderPass(screenRenderPass);
        Renderer::renderUnitQuad(screenMaterial);
        Renderer::endRenderPass();
    }

    void SceneRenderer::uiPass() {
        CG_GPU_TIME_FN(&renderingStats.uiTimer, false)

        for (const auto& [zIndex, drawInfo]: uiDrawInfoQueue) {

            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                drawInfo.textureSlots[i]->bind(i);
            }
            if (drawInfo.circleIndexCount > 0) {
                Renderer::beginRenderPass(uiCirclePass);
                Renderer::renderUiCircles(drawInfo.circleVertices, drawInfo.circleIndexCount);
                Renderer::endRenderPass();
            }
            if (drawInfo.rectIndexCount > 0) {
                Renderer::beginRenderPass(uiRectPass);
                Renderer::renderUiRects(drawInfo.rectVertices, drawInfo.rectIndexCount);
                Renderer::endRenderPass();
            }


            for (uint32_t i = 0; i < drawInfo.filledFontAtlases; i++) {
                drawInfo.fontAtlases[i]->bind(i);
            }
            if (drawInfo.textIndexCount > 0) {
                Renderer::beginRenderPass(uiTextPass);
                Renderer::renderUiText(drawInfo.textVertices, drawInfo.textIndexCount);
                Renderer::endRenderPass();
            }
        }
    }

    void SceneRenderer::clearPass(RenderPass& renderPass) {
        Renderer::beginRenderPass(renderPass);
        Renderer::endRenderPass();
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
            float shadowMapResolution = static_cast<float>(dirShadowMaps.getWidth());
            glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * shadowMapResolution / 2.0f;
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * 2.0f / shadowMapResolution;
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;

            lightProjection[3] += roundOffset;

            dirShadowData.lightSpaceMat[i] = lightProjection * lightView;
        }

        ubDirShadowData->setData(dirShadowData);
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

        ubHBAOData->setData(hbaoData);
    }

    float SceneRenderer::findDrawInfoTextureIndex(UiDrawInfo& drawInfo, const Texture2D* texture) const {
        float textureIndex = -1;
        if (texture != nullptr) {
            for (uint32_t i = 0; i < drawInfo.filledTextureSlots; i++) {
                if (*drawInfo.textureSlots[i] == *texture) {
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

    void SceneRenderer::resetRenderingStats() {
        renderingStats = {};
    }
}
