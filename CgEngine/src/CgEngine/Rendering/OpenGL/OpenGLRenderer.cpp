#include <Asserts.h>
#include <FileSystem.h>
#include <Rendering/Helpers.h>
#include "OpenGLRenderer.h"
#include "Macros.h"
#include "glad/glad.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLHelpers.h"
#include "OpenGLDescriptorSet.h"
#include "OpenGLAttachment.h"
#include "GPUDebugGroup.h"

namespace CgEngine {

    void OpenGLRenderer::init(Window& window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            glDebugMessageCallback(&OpenGLRenderer::debugCallback, nullptr);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        #endif

        const char* rendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* rendererVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* rendererVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        CG_LOGGING_INFO("RENDERER: API: OpenGL")
        CG_LOGGING_INFO("RENDERER: Vendor: {0}", rendererVendor)
        CG_LOGGING_INFO("RENDERER: Name: {0}", rendererName)
        CG_LOGGING_INFO("RENDERER: Version: {0}", rendererVersion)

        initImGui(window);

        auto unitQuadVertexData = getUnitQuadVerticesAndIndices(false);

        quadVAO = OpenGLVertexArrayObject();
        auto* quadVertexBuffer = new OpenGLVertexBuffer(std::get<0>(unitQuadVertexData).data(), std::get<0>(unitQuadVertexData).size() * sizeof(QuadVertex), VertexBufferUsage::Static);
        quadVertexBuffer->setLayout(std::get<2>(unitQuadVertexData));
        quadVAO.addVertexBuffer(quadVertexBuffer);
        quadVAO.setIndexBuffer(new OpenGLIndexBuffer(std::get<1>(unitQuadVertexData).data(), std::get<1>(unitQuadVertexData).size(), IndexBufferDataType::UInt32));

        auto unitCubeVertexData = getUnitCubeVerticesAndIndices();

        unitCubeVAO = OpenGLVertexArrayObject();
        auto* unitCubeVertexBuffer = new OpenGLVertexBuffer(std::get<0>(unitCubeVertexData).data(), std::get<0>(unitCubeVertexData).size() * sizeof(float), VertexBufferUsage::Static);
        unitCubeVertexBuffer->setLayout(std::get<2>(unitCubeVertexData));
        unitCubeVAO.addVertexBuffer(unitCubeVertexBuffer);
        unitCubeVAO.setIndexBuffer(new OpenGLIndexBuffer(std::get<1>(unitCubeVertexData).data(), std::get<1>(unitCubeVertexData).size()));

        isWireframe = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        isBackFaceCulling = true;
        isFrontFaceCulling = false;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        depthTest = true;
        glEnable(GL_DEPTH_TEST);
        depthWrite = true;
        glDepthMask(GL_TRUE);
        depthCompareOperator = DepthCompareOperator::Less;
        glDepthFunc(OpenGLHelpers::depthCompareOperatorToOpenGL(depthCompareOperator));

        useBlending = false;
        glDisable(GL_BLEND);
        blendingEquation = BlendingEquation::Add;
        glBlendEquation(OpenGLHelpers::blendingEquationToOpenGL(blendingEquation));
        srcBlendingFunction = BlendingFunction::SrcAlpha;
        destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        glBlendFunc(OpenGLHelpers::blendingFunctionToOpenGL(srcBlendingFunction), OpenGLHelpers::blendingFunctionToOpenGL(destBlendingFunction));

        tessellationPatchSize = 4;
        glPatchParameteri(GL_PATCH_VERTICES, tessellationPatchSize);

        glFrontFace(GL_CCW);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

        uint32_t whiteTextureData = 0xffffffff;
        whiteTexture = OpenGLTexture2D(TextureFormat::RGBA, 1, 1, TextureWrap::Clamp, &whiteTextureData, MipMapFiltering::Nearest);

        uint32_t blackCubeMapTextureData = 0xff000000;
        blackCubeTexture = OpenGLTextureCube(TextureFormat::RGBA, 1, 1, &blackCubeMapTextureData, MipMapFiltering::Nearest);

        brdfLUT = OpenGLTexture2D(FileSystem::getAsEnginePath("ibl_brdf_lut.png"), false, TextureWrap::Clamp, MipMapFiltering::Bilinear);

        ComputePipelineSpecification environmentMapSphereToCubeSpec{};
        environmentMapSphereToCubeSpec.engineShaderName = "sphereToCube";
        computeEnvironmentMapSphereToCube = OpenGLComputePipeline(environmentMapSphereToCubeSpec);

        ComputePipelineSpecification environmentMapPrefilterMapSpec{};
        environmentMapPrefilterMapSpec.engineShaderName = "prefilterMap";
        computeEnvironmentMapPrefilterMap = OpenGLComputePipeline(environmentMapPrefilterMapSpec);

        ComputePipelineSpecification environmentMapIrradianceMapSpec{};
        environmentMapIrradianceMapSpec.engineShaderName = "irradianceMap";
        computeEnvironmentMapIrradianceMap = OpenGLComputePipeline(environmentMapIrradianceMapSpec);

        RenderPassSpecification swapChainRenderPassSpec{};
        swapChainRenderPassSpec.clearDepthStencilAttachment = false;
        swapChainRenderPassSpec.clearColorAttachments = true;
        swapChainRenderPassSpec.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        swapChainRenderPass = OpenGLRenderPass(swapChainRenderPassSpec);

        swapChainFramebuffer = OpenGLFramebuffer(window.getFramebufferWidth(), window.getFramebufferHeight(), true);
        glfwSwapInterval(window.isVsync() ? 1 : 0);

        glCreateFramebuffers(1, &dynamicRenderingFramebufferHandle);

        glCreateBuffers(1, &pushConstantsBuffer);
        glNamedBufferStorage(pushConstantsBuffer, 128 * sizeof(std::byte), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, pushConstantsBuffer);
    }

    void OpenGLRenderer::shutdown() {
        glDeleteFramebuffers(1, &dynamicRenderingFramebufferHandle);
        glDeleteBuffers(1, &pushConstantsBuffer);
        shutdownImGui();
    }

    void OpenGLRenderer::setFramebufferResized() {
        framebufferResized = true;
    }

    bool OpenGLRenderer::beginFrame(const Window& window) {
        if (window.getFramebufferWidth() <= 0 || window.getFramebufferHeight() <= 0) {
            return false;
        }

        if (framebufferResized) {
           swapChainFramebuffer.recreate(window.getFramebufferWidth(), window.getFramebufferHeight());
        }

        framebufferResized = false;
        frameIndex++;

        return true;
    }

    void OpenGLRenderer::endFrame(const Window& window) {
        glfwSwapBuffers(&window.getWindowHandle());
    }

    void OpenGLRenderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")
        CG_ASSERT(!currentlyDynamicRendering, "Cannot begin regular RenderPass while in dynamic rendering!")

        currentRenderPass = static_cast<const OpenGLRenderPass*>(renderPass);
        const RenderPassSpecification& spec = currentRenderPass->getSpecification();

        auto* fb = static_cast<const OpenGLFramebuffer*>(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->getOpenGLHandle());
        glViewport(0, 0, static_cast<int>(fb->getWidth()), static_cast<int>(fb->getHeight()));

        if (spec.clearColorAttachments) {
            const glm::vec4& clearColor = spec.clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (spec.clearDepthStencilAttachment) {
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            if (fb->hasStencilAttachment()) {
                glClear(GL_STENCIL_BUFFER_BIT);
            }
            depthWrite = true;
        }
    }

    void OpenGLRenderer::beginSwapChainRenderPass() {
        beginRenderPass(&swapChainRenderPass, &swapChainFramebuffer);
    }

    void OpenGLRenderer::endRenderPass() {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")
        CG_ASSERT(!currentlyDynamicRendering, "Cannot end regular RenderPass while in dynamic rendering!")
        currentRenderPass = nullptr;
        currentPipelineHandle = ~0;
    }

    void OpenGLRenderer::beginDynamicRendering(const DynamicRenderingInfo& renderingInfo) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")
        CG_ASSERT(!currentlyDynamicRendering, "Already in dynamic rendering!")

        currentlyDynamicRendering = true;

        glBindFramebuffer(GL_FRAMEBUFFER, dynamicRenderingFramebufferHandle);

        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(renderingInfo.colorAttachments.size());

        for (size_t i = 0; i < renderingInfo.colorAttachments.size(); i++) {
            const auto* attachment = static_cast<const OpenGLAttachment*>(renderingInfo.colorAttachments[i].attachment);

            if (renderingInfo.colorAttachments[i].allLayers) {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            } else {
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, attachment->getOpenGLHandle(), 0, renderingInfo.colorAttachments[i].layer);
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
        }

        if (!drawBuffers.empty()) {
            glDrawBuffers(static_cast<int>(drawBuffers.size()), drawBuffers.data());
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        bool hasStencil = false;

        if (renderingInfo.depthStencilAttachment.attachment != nullptr) {
            const auto* depthAttachment = static_cast<const OpenGLAttachment*>(renderingInfo.depthStencilAttachment.attachment);
            CG_ASSERT(depthAttachment->getType() == AttachmentType::Depth || depthAttachment->getType() == AttachmentType::DepthStencil, "DepthStencil attachment must be of type Depth or DepthStencil")
            if (depthAttachment->getType() == AttachmentType::DepthStencil) {
                if (renderingInfo.depthStencilAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachment->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachment->getOpenGLHandle(), 0, renderingInfo.depthStencilAttachment.layer);
                }
                hasStencil = true;
            } else {
                if (renderingInfo.depthStencilAttachment.allLayers) {
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachment->getOpenGLHandle(), 0);
                } else {
                    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachment->getOpenGLHandle(), 0, renderingInfo.depthStencilAttachment.layer);
                }
            }
        }

        glViewport(0, 0, static_cast<int>(renderingInfo.renderArea.x), static_cast<int>(renderingInfo.renderArea.y));

        if (renderingInfo.clearColorAttachments) {
            const glm::vec4& clearColor = renderingInfo.clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (renderingInfo.clearDepthStencilAttachment) {
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            if (hasStencil) {
                glClear(GL_STENCIL_BUFFER_BIT);
            }
            depthWrite = true;
        }
    }

    void OpenGLRenderer::endDynamicRendering() {
        CG_ASSERT(currentRenderPass == nullptr, "There is a active RenderPass!")
        CG_ASSERT(currentlyDynamicRendering, "Not in dynamic rendering!")
        currentlyDynamicRendering = false;
        currentPipelineHandle = ~0;
    }

    void OpenGLRenderer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering has not been started!")

        auto* glGraphicsPipeline = static_cast<const OpenGLGraphicsPipeline*>(graphicsPipeline);
        const GraphicsPipelineSpecification& spec = glGraphicsPipeline->getSpecification();
        currentPipelineHandle = glGraphicsPipeline->getOpenGLShaderHandle();
        drawMode = glGraphicsPipeline->getDrawMode();

        CG_ASSERT(currentRenderPass != &swapChainRenderPass || (spec.depthTest == false && spec.depthWrite == false), "SwapChain RenderPass must have depth testing and writing disabled!")

        glUseProgram(glGraphicsPipeline->getOpenGLShaderHandle());

        if (isWireframe != spec.wireframe) {
            isWireframe = spec.wireframe;
            if (isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        if (isBackFaceCulling != spec.backfaceCulling || isFrontFaceCulling != spec.frontfaceCulling) {
            isBackFaceCulling = spec.backfaceCulling;
            isFrontFaceCulling = spec.frontfaceCulling;
            if (isBackFaceCulling || isFrontFaceCulling) {
                glEnable(GL_CULL_FACE);
            } else {
                glDisable(GL_CULL_FACE);
            }
            if (isBackFaceCulling) {
                glCullFace(GL_BACK);
            }
            if (isFrontFaceCulling) {
                glCullFace(GL_FRONT);
            }
        }
        if (depthTest != spec.depthTest) {
            depthTest = spec.depthTest;
            if (depthTest) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
        }
        if (depthWrite != spec.depthWrite) {
            depthWrite = spec.depthWrite;
            if (depthWrite) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }
        }
        if (depthCompareOperator != spec.depthCompareOperator) {
            depthCompareOperator = spec.depthCompareOperator;
            glDepthFunc(OpenGLHelpers::depthCompareOperatorToOpenGL(spec.depthCompareOperator));
        }
        if (useBlending != spec.useBlending) {
            useBlending = spec.useBlending;
            if (useBlending) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
        }
        if (blendingEquation != spec.blendingEquation) {
            blendingEquation = spec.blendingEquation;
            glBlendEquation(OpenGLHelpers::blendingEquationToOpenGL(blendingEquation));
        }
        if (srcBlendingFunction != spec.srcBlendingFunction || destBlendingFunction != spec.destBlendingFunction) {
            srcBlendingFunction = spec.srcBlendingFunction;
            destBlendingFunction = spec.destBlendingFunction;
            glBlendFunc(OpenGLHelpers::blendingFunctionToOpenGL(srcBlendingFunction), OpenGLHelpers::blendingFunctionToOpenGL(destBlendingFunction));
        }

        if (spec.drawMode == DrawMode::Patches && spec.tesselationPatchSize != tessellationPatchSize) {
            tessellationPatchSize = spec.tesselationPatchSize;
            glPatchParameteri(GL_PATCH_VERTICES, tessellationPatchSize);
        }
    }

    void OpenGLRenderer::bindComputePipeline(const ComputePipeline* computePipeline) {
        auto* glComputePipeline = static_cast<const OpenGLComputePipeline*>(computePipeline);
        glUseProgram(glComputePipeline->getOpenGLShaderHandle());
        currentPipelineHandle = glComputePipeline->getOpenGLShaderHandle();
    }

    void OpenGLRenderer::dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        glDispatchCompute(groupCountX, groupCountY, groupCountZ);
    }

    void OpenGLRenderer::clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")
        CG_ASSERT(!currentlyDynamicRendering, "Cannot clear while in dynamic rendering!")

        auto* fb = static_cast<const OpenGLFramebuffer*>(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->getOpenGLHandle());
        glViewport(0, 0, static_cast<int>(fb->getWidth()), static_cast<int>(fb->getHeight()));

        const RenderPassSpecification& spec = static_cast<const OpenGLRenderPass*>(renderPass)->getSpecification();

        if (spec.clearColorAttachments) {
            const glm::vec4& clearColor = spec.clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (spec.clearDepthStencilAttachment) {
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            if (fb->hasStencilAttachment()) {
                glClear(GL_STENCIL_BUFFER_BIT);
            }
            depthWrite = true;
        }
    }

    void OpenGLRenderer::bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) {
        const auto* glDescriptorSet = static_cast<const OpenGLDescriptorSet*>(descriptorSet);
        glDescriptorSet->bind();
    }

    void OpenGLRenderer::setPushConstants(const void* data, size_t size) {
        CG_ASSERT(size <= 128 * sizeof(std::byte), "Push constant data size exceeds the maximum allowed size of 128 bytes.")
        glNamedBufferSubData(pushConstantsBuffer, 0, size, data);
    }

    void OpenGLRenderer::injectBarriersForDescriptorSet(const DescriptorSet *descriptorSet) {

    }

    void OpenGLRenderer::transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) {
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    void OpenGLRenderer::memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) {
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    }

    void OpenGLRenderer::memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment) {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void OpenGLRenderer::renderUnitQuad() {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering!")
        CG_ASSERT(currentPipelineHandle != ~0, "There is no active GraphicsPipeline!")

        quadVAO.bind();
        glDrawElements(GL_TRIANGLES, quadVAO.getIndexBuffer()->getIndexCount(), OpenGLHelpers::getOpenGLIndexType(quadVAO.getIndexBuffer()->getDataType()), nullptr);
    }

    void OpenGLRenderer::renderUnitCube() {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering!")
        CG_ASSERT(currentPipelineHandle != ~0, "There is no active GraphicsPipeline!")

        unitCubeVAO.bind();
        glDrawElements(GL_TRIANGLES, unitCubeVAO.getIndexBuffer()->getIndexCount(), OpenGLHelpers::getOpenGLIndexType(unitCubeVAO.getIndexBuffer()->getDataType()), nullptr);
    }

    void OpenGLRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering!")
        CG_ASSERT(currentPipelineHandle != ~0, "There is no active GraphicsPipeline!")

        auto* glVao = static_cast<const OpenGLVertexArrayObject*>(vao);

        glVao->bind();
        glDrawElementsInstancedBaseVertex(drawMode, indexCount, OpenGLHelpers::getOpenGLIndexType(glVao->getIndexBuffer()->getDataType()), (void*)(baseIndex * sizeof(uint32_t)), instanceCount, baseVertex);
    }

    void OpenGLRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering!")
        CG_ASSERT(currentPipelineHandle != ~0, "There is no active GraphicsPipeline!")

        auto* glVao = static_cast<const OpenGLVertexArrayObject*>(vao);

        glVao->bind();
        glDrawElementsBaseVertex(drawMode, indexCount, OpenGLHelpers::getOpenGLIndexType(glVao->getIndexBuffer()->getDataType()), (void*)(baseIndex * sizeof(uint32_t)), baseVertex);
    }

    void OpenGLRenderer::drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) {
        CG_ASSERT(currentRenderPass != nullptr || currentlyDynamicRendering, "There is no active RenderPass or dynamic rendering!")
        CG_ASSERT(currentPipelineHandle != ~0, "There is no active GraphicsPipeline!")

        auto* glVao = static_cast<const OpenGLVertexArrayObject*>(vao);

        glVao->bind();
        glDrawArrays(drawMode, 0, vertexCount);
    }

    Texture2D* OpenGLRenderer::getWhiteTexture() {
        return &whiteTexture;
    }

    Texture2D* OpenGLRenderer::getBrdfLUTTexture() {
        return &brdfLUT;
    }

    TextureCube* OpenGLRenderer::getBlackCubeTexture() {
        return &blackCubeTexture;
    }

    std::pair<TextureCube*, TextureCube*> OpenGLRenderer::createEnvironmentMap(const std::string& hdriPath) {
        CG_LOGGING_DEBUG("Creating Environment Map from: {0}", hdriPath)

        const uint32_t MAP_SIZE = 1024;

        OpenGLTexture2D sphereMap("assets/game/" + hdriPath, false);

        OpenGLTextureCube cubeMap(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Bilinear);

        glUseProgram(computeEnvironmentMapSphereToCube.getOpenGLShaderHandle());
        glBindTextureUnit(0, sphereMap.getOpenGLHandle());
        glBindImageTexture(1, cubeMap.getOpenGLHandle(), 0, GL_TRUE, 0, OpenGLHelpers::shaderImageAccessToOpenGL(ShaderImageAccess::WriteOnly), OpenGLHelpers::getOpenGLTextureFormatForImageBind(cubeMap.getFormat()));
        glDispatchCompute(MAP_SIZE / 32, MAP_SIZE / 32, 6);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        cubeMap.generateMipMaps();
        OpenGLHelpers::applyMipMapFiltering(MipMapFiltering::Trilinear, GL_TEXTURE_CUBE_MAP);

        uint32_t mipCount = Helpers::calculateMipCount(MAP_SIZE, MAP_SIZE);

        auto* prefilterMap = new OpenGLTextureCube(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Trilinear);
        prefilterMap->generateMipMaps();

        struct PrefilterPushConstants {
            float roughness;
        } prefilterPushConstantsData{};


        glUseProgram(computeEnvironmentMapPrefilterMap.getOpenGLShaderHandle());
        glBindTextureUnit(0, cubeMap.getOpenGLHandle());

        for (uint32_t i = 0, size = MAP_SIZE; i < mipCount; i++, size /= 2) {
            uint32_t numGroups = glm::max(1u, size / 32);
            float roughness = static_cast<float>(i) / static_cast<float>(mipCount - 1);
            prefilterPushConstantsData.roughness = roughness;
            setPushConstants(&prefilterPushConstantsData, sizeof(PrefilterPushConstants));
            glBindImageTexture(1, prefilterMap->getOpenGLHandle(), static_cast<int>(i), GL_TRUE, 0, OpenGLHelpers::shaderImageAccessToOpenGL(ShaderImageAccess::WriteOnly), OpenGLHelpers::getOpenGLTextureFormatForImageBind(prefilterMap->getFormat()));
            glDispatchCompute(numGroups, numGroups, 6);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }

        auto* irradianceMap = new OpenGLTextureCube(TextureFormat::Float32A, 32, 32, MipMapFiltering::Bilinear);

        glUseProgram(computeEnvironmentMapIrradianceMap.getOpenGLShaderHandle());
        glBindTextureUnit(0, prefilterMap->getOpenGLHandle());
        glBindImageTexture(1, irradianceMap->getOpenGLHandle(), 0, GL_TRUE, 0, OpenGLHelpers::shaderImageAccessToOpenGL(ShaderImageAccess::WriteOnly), OpenGLHelpers::getOpenGLTextureFormatForImageBind(irradianceMap->getFormat()));
        glDispatchCompute(irradianceMap->getWidth() / 2, irradianceMap->getWidth() / 2, 6);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        return {irradianceMap, prefilterMap};
    }

    const std::vector<VertexBufferLayout> OpenGLRenderer::getUnitQuadVertexInputLayout() {
        return quadVAO.getLayout();
    }

    const std::vector<VertexBufferLayout> OpenGLRenderer::getUnitCubeVertexInputLayout() {
        return unitCubeVAO.getLayout();
    }

    PipelineAttachmentInfo OpenGLRenderer::getSwapChainAttachmentInfo() {
        PipelineAttachmentInfo info{};
        info.colorAttachments = { AttachmentType::RGBA8 };
        info.hasDepthStencilAttachment = true;
        info.depthAttachmentFormat = DepthStencilAttachmentFormat::Depth24Stencil8;

        return info;
    }

    void OpenGLRenderer::beginImGuiFrame() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        #endif
    }

    void OpenGLRenderer::renderImGuiFrame() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            CG_GPU_DEBUG_GROUP("ImGui")

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        #endif
    }

    uint64_t OpenGLRenderer::getFrameIndex() {
        return frameIndex;
    }

    void OpenGLRenderer::initImGui(Window& window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            glm::vec2 contentScale = window.getContentScale();

            ImGui::StyleColorsDark();
            ImGui::GetStyle().ScaleAllSizes(glm::max(contentScale.x, contentScale.y));

            ImGui_ImplGlfw_InitForOpenGL(&window.getWindowHandle(), true);
            ImGui_ImplOpenGL3_Init("#version 450 core");
        #endif
    }

    void OpenGLRenderer::shutdownImGui() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        #endif
    }

    void OpenGLRenderer::debugCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam) {
        if (id == 131185 || id == 131218 || (source == GL_DEBUG_SOURCE_APPLICATION && id == 0)) {
            return; // ignore performance warnings (buffer uses GPU memory, shader recompilation) from nvidia
        }

        std::stringstream stringStream;
        std::string sourceString;
        std::string typeString;
        std::string severityString;

        switch (source) {
            case GL_DEBUG_SOURCE_API: {
                sourceString = "API";
                break;
            }
            case GL_DEBUG_SOURCE_APPLICATION: {
                sourceString = "Application";
                break;
            }
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
                sourceString = "Window System";
                break;
            }
            case GL_DEBUG_SOURCE_SHADER_COMPILER: {
                sourceString = "Shader Compiler";
                break;
            }
            case GL_DEBUG_SOURCE_THIRD_PARTY: {
                sourceString = "Third Party";
                break;
            }
            case GL_DEBUG_SOURCE_OTHER: {
                sourceString = "Other";
                break;
            }
            default: {
                sourceString = "Unknown";
                break;
            }
        }

        switch (type) {
            case GL_DEBUG_TYPE_ERROR: {
                typeString = "Error";
                break;
            }
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
                typeString = "Deprecated Behavior";
                break;
            }
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
                typeString = "Undefined Behavior";
                break;
            }
            case GL_DEBUG_TYPE_PERFORMANCE: {
                typeString = "Performance";
                break;
            }
            case GL_DEBUG_TYPE_OTHER: {
                typeString = "Other";
                break;
            }
            default: {
                typeString = "Unknown";
                break;
            }
        }

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: {
                severityString = "High";
                break;
            }
            case GL_DEBUG_SEVERITY_MEDIUM: {
                severityString = "Medium";
                break;
            }
            case GL_DEBUG_SEVERITY_LOW: {
                severityString = "Low";
                break;
            }
            default: {
                severityString = "Unknown";
                break;
            }
        }

        stringStream << "OpenGL Error: " << message;
        stringStream << " [Source = " << sourceString;
        stringStream << ", Type = " << typeString;
        stringStream << ", Severity = " << severityString;
        stringStream << ", ID = " << id << "]";

        CG_LOGGING_WARNING(stringStream.str());
    }
}
