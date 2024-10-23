#include "SceneRenderer.h"
#include "Asserts.h"
#include "Application.h"

namespace CgEngine {
    SceneRenderer::SceneRenderer(uint32_t viewportWidth, uint32_t viewportHeight) : viewportWidth(viewportWidth), viewportHeight(viewportHeight) {
        {
            ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

            dirShadowMaps = new Texture2DArray(TextureFormat::Depth, applicationOptions.shadowMapResolution, applicationOptions.shadowMapResolution, TextureWrap::ClampBorder, 4, MipMapFiltering::Bilinear);
            dirShadowMaps->setClampBorderColor({1.0f, 1.0f, 1.0f, 1.0f});

            FramebufferSpecification shadowMapFramebufferSpec;
            shadowMapFramebufferSpec.height = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.width = applicationOptions.shadowMapResolution;
            shadowMapFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            shadowMapFramebufferSpec.hasDepthStencilAttachment = false;
            shadowMapFramebufferSpec.hasDepthAttachment = false;
            shadowMapFramebufferSpec.useExistingDepthAttachment = true;
            shadowMapFramebufferSpec.existingDepthAttachment = dirShadowMaps->getRendererId();

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
        }
        {
            FramebufferSpecification preDepthFramebufferSpec;
            preDepthFramebufferSpec.height = viewportHeight;
            preDepthFramebufferSpec.width = viewportWidth;
            preDepthFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
            preDepthFramebufferSpec.hasDepthStencilAttachment = false;
            preDepthFramebufferSpec.hasDepthAttachment = true;

            auto* framebuffer = new Framebuffer(preDepthFramebufferSpec);

            RenderPassSpecification preDepthRenderPassSpec;
            preDepthRenderPassSpec.shader = Shader("preDepth");
            preDepthRenderPassSpec.framebuffer = framebuffer;
            preDepthRenderPassSpec.clearColorBuffer = false;
            preDepthRenderPassSpec.clearDepthBuffer = true;
            preDepthRenderPassSpec.depthCompareOperator = DepthCompareOperator::Less;

            preDepthRenderPass = RenderPass(std::move(preDepthRenderPassSpec));
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
        }
        {
            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;
            for (auto& bloomTexture: bloomTextures) {
                bloomTexture = new Texture2D(TextureFormat::Float32, static_cast<uint32_t>(bloomWidth), static_cast<uint32_t>(bloomHeight), TextureWrap::Clamp, MipMapFiltering::Bilinear);
                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }

            FramebufferSpecification bloomFramebufferSpec;
            bloomFramebufferSpec.width = viewportWidth / 2;
            bloomFramebufferSpec.height = viewportHeight / 2;
            bloomFramebufferSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            bloomFramebufferSpec.hasDepthStencilAttachment = false;
            bloomFramebufferSpec.useExistingColorAttachment = true;
            bloomFramebufferSpec.existingColorAttachment = bloomTextures[0]->getRendererId();

            auto* framebuffer = new Framebuffer(bloomFramebufferSpec);

            RenderPassSpecification bloomDownSamplePassSpec;
            bloomDownSamplePassSpec.shader = Shader("bloomDownSample");
            bloomDownSamplePassSpec.clearColorBuffer = true;
            bloomDownSamplePassSpec.clearDepthBuffer = false;
            bloomDownSamplePassSpec.depthTest = false;
            bloomDownSamplePassSpec.depthWrite = false;
            bloomDownSamplePassSpec.framebuffer = framebuffer;

            bloomDownSamplePass = RenderPass(std::move(bloomDownSamplePassSpec));

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

            screenMaterial.setTexture("u_FinalImage", geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
            screenMaterial.setTexture("u_BloomTexture", bloomTextures[0]->getRendererId(), 1);
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

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

        for (uint32_t i = 0; i < Renderer::maxTextureSlots; i++) {
            Renderer::getWhiteTexture().bind(i);
        }

        ubCameraData = new UniformBuffer<UBCameraData>("CameraData", 0, geometryRenderPass.getSpecification().shader);
        ubLightData = new UniformBuffer<UBLightData>("LightData", 1, geometryRenderPass.getSpecification().shader);
        ubDirShadowData = new UniformBuffer<UBDirShadowData>("DirShadowData", 2, shadowMapRenderPass.getSpecification().shader);

        boneTransformsBuffer = new ShaderStorageBuffer();
        boneTransformsBuffer->setData(nullptr, maxBones * maxAnimatedComponents * sizeof(glm::mat4));

        skinningShader = ComputeShader("skinning");
    }

    SceneRenderer::~SceneRenderer() {
        delete dirShadowMaps;

        for (int i = 0; i < bloomTextures.size(); i++) {
            delete bloomTextures[i];
        }

        delete ubCameraData;
        delete ubLightData;
        delete ubDirShadowData;

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
        }
    }

    void SceneRenderer::beginScene(const Camera& camera, glm::mat4 cameraTransform, const SceneLightEnvironment& lightEnvironment, const SceneEnvironment& sceneEnvironment) {
        CG_ASSERT(!activeRendering, "Already Rendering Scene!")
        CG_ASSERT(activeScene, "No active Scene!")

#ifdef CG_ENABLE_DEBUG_FEATURES
        sceneIndex++;
#endif

        activeRendering = true;

        if (needsResize && viewportWidth != 0 && viewportHeight != 0) {
            needsResize = false;

            preDepthRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);
            geometryRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);
            geometryRenderPass.getSpecification().framebuffer->setDepthAttachment(preDepthRenderPass.getSpecification().framebuffer->getDepthAttachmentRendererId(), 0, viewportWidth, viewportHeight);
            screenRenderPass.getSpecification().framebuffer->resize(viewportWidth, viewportHeight, false);

            float bloomWidth = static_cast<float>(viewportWidth) / 2.0f;
            float bloomHeight = static_cast<float>(viewportHeight) / 2.0f;
            for (int i = 0; i < bloomTextures.size(); i++) {
                delete bloomTextures[i];
                bloomTextures[i] = new Texture2D(TextureFormat::Float32, static_cast<uint32_t>(bloomWidth), static_cast<uint32_t>(bloomHeight), TextureWrap::Clamp, MipMapFiltering::Bilinear);
                bloomWidth /= 2.0f;
                bloomHeight /= 2.0f;
            }

            screenMaterial.setTexture("u_FinalImage", geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
            screenMaterial.setTexture("u_BloomTexture", bloomTextures[0]->getRendererId(), 1);

            uiProjectionMatrix = glm::ortho(0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight));
        }

        UBCameraData cameraData{};
        cameraData.projection = camera.getProjectionMatrix();
        cameraData.view = glm::inverse(cameraTransform);
        cameraData.viewProjection = cameraData.projection * cameraData.view;
        cameraData.uiProjectionMatrix = uiProjectionMatrix;
        cameraData.position = cameraTransform[3];
        cameraData.exposure = camera.getExposure();
        cameraData.bloomIntensity = camera.getBloomIntensity();
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
    }

    void SceneRenderer::endScene() {
        CG_ASSERT(activeRendering, "Not actively rendering!")

        ApplicationOptions& applicationOptions = Application::get().getApplicationOptions();

        skinMeshes();
        shadowMapPass();
        preDepthPass();
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

    void SceneRenderer::submitCustomShaderMesh(Mesh* mesh, const std::vector<uint32_t>& meshNodes, Material* material, bool enableCulling, AABoundingBox& boundingBox, const glm::mat4& transform, CustomShader* shader, uint32_t instanceCount, CustomShaderRendererComponentRenderPassOptions& renderPassOptions, ShaderStorageBuffer* instanceBuffer) {
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
                drawCommand.instanceBuffer = instanceBuffer;
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

    void SceneRenderer::skinMeshes() {
        skinningShader.bind();
        boneTransformsBuffer->bind(2);

        for (uint32_t i = 0; i < skinningQueue.size(); i++) {
            skinningQueue[i].originalVertexBuffer->bindAsSSBO(3);
            skinningQueue[i].skinnedVertexBuffer->bindAsSSBO(4);
            skinningQueue[i].boneInfluencesBuffer->bind(1);

            skinningShader.setInt("u_ComponentIndex", i);
            skinningShader.dispatch((skinningQueue[i].numVertices / 32) + 1, 1, 1);
            skinningShader.waitForMemoryBarrier();
        }
    }

    void SceneRenderer::shadowMapPass() {
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
        Renderer::beginRenderPass(preDepthRenderPass);

        for (const auto [mk, command]: drawCommandQueue) {
            const auto& transforms = meshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, emptyMaterial, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::geometryPass() {
        Renderer::beginRenderPass(geometryRenderPass);

        geometryRenderPass.getSpecification().shader.setTexture(currentSceneEnvironment.irradianceMapId, 5);
        geometryRenderPass.getSpecification().shader.setTexture(currentSceneEnvironment.prefilterMapId, 6);
        geometryRenderPass.getSpecification().shader.setTexture(Renderer::getBrdfLUTTexture().getRendererId(), 7);
        geometryRenderPass.getSpecification().shader.setFloat("u_EnvironmentIntensity", currentSceneEnvironment.environmentIntensity);
        geometryRenderPass.getSpecification().shader.setTexture(dirShadowMaps->getRendererId(), 8);

        for (const auto [mk, command]: drawCommandQueue) {
            const auto& transforms = meshTransforms[mk];
            Renderer::executeDrawCommand(*command.vao, *command.material, command.indexCount, command.baseIndex, command.baseVertex, transforms, command.instanceCount);
        }

        Renderer::endRenderPass();
    }

    void SceneRenderer::customShaderPass() {
        Renderer::beginRenderPass(customShaderRenderPass, true);

        // can be true at the start, because the values are already bound from the previous geometryPass
        bool lastCommandUseDirShadowMappingData = true;
        bool lastCommandUseEnvironmentMappingData = true;

        const Material* lastUsedMaterial = nullptr;

        for (const auto& [shader, commands]: customShaderDrawCommandQueue) {
            shader->bind();

            for (const auto& command: commands) {
                shader->setMat4("u_Transform", command.transform);

                if (command.instanceBuffer != nullptr) {
                    command.instanceBuffer->bind(5);
                }

                if (command.renderPassOptions.useDirShadowMappingData && !lastCommandUseDirShadowMappingData) {
                    shader->setTexture(dirShadowMaps->getRendererId(), 8);
                }
                lastCommandUseDirShadowMappingData = command.renderPassOptions.useDirShadowMappingData;

                if (command.renderPassOptions.useEnvironmentMappingData && !lastCommandUseEnvironmentMappingData) {
                    shader->setTexture(currentSceneEnvironment.irradianceMapId, 5);
                    shader->setTexture(currentSceneEnvironment.prefilterMapId, 6);
                    shader->setTexture(Renderer::getBrdfLUTTexture().getRendererId(), 7);
                    shader->setFloat("u_EnvironmentIntensity", currentSceneEnvironment.environmentIntensity);
                    shader->setTexture(dirShadowMaps->getRendererId(), 8);
                }
                lastCommandUseEnvironmentMappingData = command.renderPassOptions.useEnvironmentMappingData;

                const Material* material = command.material != nullptr ? command.material : &emptyMaterial;

                Renderer::setFaceCulling(command.renderPassOptions.backfaceCulling, command.renderPassOptions.backfaceCulling);
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
        auto& downSampleShader = bloomDownSamplePass.getSpecification().shader;

        bloomDownSamplePass.getSpecification().framebuffer->setColorAttachment(bloomTextures[0]->getRendererId(), 0, viewportWidth / 2, viewportHeight / 2);
        Renderer::beginRenderPass(bloomDownSamplePass);
        downSampleShader.setTexture(geometryRenderPass.getSpecification().framebuffer->getColorAttachmentRendererId(0), 0);
        downSampleShader.setBool("u_UseThreshold", true);
        Renderer::renderUnitQuad(emptyMaterial);
        Renderer::endRenderPass();

        downSampleShader.setBool("u_UseThreshold", false);
        for (uint32_t i = 0; i < bloomTextures.size() - 1; ++i) {
            bloomDownSamplePass.getSpecification().framebuffer->setColorAttachment(bloomTextures[i + 1]->getRendererId(), 0, bloomTextures[i + 1]->getWidth(), bloomTextures[i + 1]->getHeight());
            Renderer::beginRenderPass(bloomDownSamplePass);
            downSampleShader.setTexture(bloomTextures[i]->getRendererId(), 0);
            Renderer::renderUnitQuad(emptyMaterial);
            Renderer::endRenderPass();
        }

        auto& upSampleShader = bloomUpSamplePass.getSpecification().shader;

        for (uint32_t i = bloomTextures.size() - 1; i > 0; i--) {
            bloomUpSamplePass.getSpecification().framebuffer->setColorAttachment(bloomTextures[i - 1]->getRendererId(), 0, bloomTextures[i - 1]->getWidth(), bloomTextures[i - 1]->getHeight());
            Renderer::beginRenderPass(bloomUpSamplePass);
            upSampleShader.setTexture(bloomTextures[i]->getRendererId(), 0);
            Renderer::renderUnitQuad(emptyMaterial);
            Renderer::endRenderPass();
        }
    }

    void SceneRenderer::screenPass() {
        Renderer::beginRenderPass(screenRenderPass);
        Renderer::renderUnitQuad(screenMaterial);
        Renderer::endRenderPass();
    }

    void SceneRenderer::uiPass() {
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
            float shadowMapResolution = static_cast<float>(dirShadowMaps->getWidth());
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
}
