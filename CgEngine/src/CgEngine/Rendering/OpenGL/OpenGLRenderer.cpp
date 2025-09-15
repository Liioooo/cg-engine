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
#include "OpenGLPushConstants.h"

namespace CgEngine {

    void OpenGLRenderer::init(Window& window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            glDebugMessageCallback(&OpenGLRenderer::debugCallback, nullptr);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        #endif

        const char* rendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* rendererVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* rendererVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        CG_LOGGING_INFO("RENDERER: Vendor: {0}", rendererVendor)
        CG_LOGGING_INFO("RENDERER: Name: {0}", rendererName)
        CG_LOGGING_INFO("RENDERER: Version: {0}", rendererVersion)

        initImGui(window);

        struct QuadVertex {
            glm::vec3 pos;
            glm::vec2 uv;
        };
        QuadVertex quadVertexData[4];
        quadVertexData[0].pos = {-1.0f, -1.0f, 0.0f};
        quadVertexData[0].uv = {0.0f, 0.0f};

        quadVertexData[1].pos = {1.0f, -1.0f, 0.0f};
        quadVertexData[1].uv = {1.0f, 0.0f};

        quadVertexData[2].pos = {1.0f, 1.0f, 0.0f};
        quadVertexData[2].uv = {1.0f, 1.0f};

        quadVertexData[3].pos = {-1.0f, 1.0f, 0.0f};
        quadVertexData[3].uv = {0.0f, 1.0f};

        quadVAO = OpenGLVertexArrayObject();
        auto* quadVertexBuffer = new OpenGLVertexBuffer(quadVertexData, 4 * sizeof(QuadVertex), VertexBufferUsage::Static);
        quadVertexBuffer->setLayout({{ShaderDataType::Float3, true}, {ShaderDataType::Float2, true}});
        quadVAO.addVertexBuffer(quadVertexBuffer);
        uint32_t quadIndices[6] = {0, 1, 2, 2, 3, 0 };
        quadVAO.setIndexBuffer(new OpenGLIndexBuffer(quadIndices, 6, IndexBufferDataType::UInt32));

        float unitCubeVertices[] = {
                -1.0f, 1.0f, 1.0f, // left_top_front_0
                -1.0f, -1.0f, 1.0f, // left_bottom_front_1
                1.0f, 1.0f, 1.0f, // right_top_front_2
                1.0f, -1.0f, 1.0f, // right_bottom_front_3
                1.0f, 1.0f, -1.0f, // right_top_back_4
                1.0f, -1.0f, -1.0f, // right_bottom_back_5
                -1.0f, 1.0f, -1.0f, // left_top_back_6
                -1.0f, -1.0f, -1.0f, // left_bottom_back_7
        };

        uint32_t unitCubeIndices[] = {
                7, 5, 4,
                4, 6, 7,
                3, 2, 4,
                4, 5, 3,
                1, 7, 0,
                0, 7, 6,
                0, 2, 3,
                3, 1, 0,
                6, 4, 0,
                0, 4, 2,
                1, 3, 7,
                5, 7, 3
        };

        unitCubeVAO = OpenGLVertexArrayObject();
        auto* unitCubeVertexBuffer = new OpenGLVertexBuffer(unitCubeVertices, sizeof(unitCubeVertices));
        unitCubeVertexBuffer->setLayout({{ShaderDataType::Float3, false}});
        unitCubeVAO.addVertexBuffer(unitCubeVertexBuffer);
        unitCubeVAO.setIndexBuffer(new OpenGLIndexBuffer(unitCubeIndices, 36));

        linesVAO = OpenGLVertexArrayObject();
        auto* linesVertexBuffer = new OpenGLVertexBuffer(0, VertexBufferUsage::Dynamic);
        linesVertexBuffer->setLayout({{ShaderDataType::Float3, false}, {ShaderDataType::Float3, false}});
        linesVAO.addVertexBuffer(linesVertexBuffer);

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

        uiIndexBuffer = OpenGLIndexBuffer(uiIndices, MAX_UI_INDICES, IndexBufferDataType::UInt32);

        uiCircleVAO = OpenGLVertexArrayObject();
        auto* uiCircleVertexBuffer = new OpenGLVertexBuffer(0, VertexBufferUsage::Dynamic);
        uiCircleVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
        uiCircleVAO.addVertexBuffer(uiCircleVertexBuffer);
        uiCircleVAO.useExistingIndexBuffer(&uiIndexBuffer);

        uiRectVAO = OpenGLVertexArrayObject();
        auto* uiRectVertexBuffer = new OpenGLVertexBuffer(0, VertexBufferUsage::Dynamic);
        uiRectVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float2, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
        uiRectVAO.addVertexBuffer(uiRectVertexBuffer);
        uiRectVAO.useExistingIndexBuffer(&uiIndexBuffer);

        uiTextVAO = OpenGLVertexArrayObject();
        auto* uiTextVertexBuffer = new OpenGLVertexBuffer(0, VertexBufferUsage::Dynamic);
        uiTextVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}});
        uiTextVAO.addVertexBuffer(uiTextVertexBuffer);
        uiTextVAO.useExistingIndexBuffer(&uiIndexBuffer);

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

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        uint32_t whiteTextureData = 0xffffffff;
        whiteTexture = OpenGLTexture2D(TextureFormat::RGBA, 1, 1, TextureWrap::Clamp, &whiteTextureData, MipMapFiltering::Nearest, 1.0f, true);

        uint32_t blackCubeMapTextureData = 0xff000000;
        blackCubeTexture = OpenGLTextureCube(TextureFormat::RGBA, 1, 1, &blackCubeMapTextureData, MipMapFiltering::Nearest);

        brdfLUT = OpenGLTexture2D(FileSystem::getAsEnginePath("ibl_brdf_lut.png"), false, TextureWrap::Clamp, MipMapFiltering::Bilinear);

//        transformsBuffer = ShaderStorageBuffer();
//        transformsBuffer.bind(0);
//
//        environmentMapSphereToCube = ComputeShader("sphereToCube");
//        environmentMapPrefilterMap = ComputeShader("prefilterMap");
//        environmentMapIrradianceMap = ComputeShader("irradianceMap");
    }

    void OpenGLRenderer::shutdown() {
        shutdownImGui();
    }

    void OpenGLRenderer::beginFrame(const Window& window) {

    }

    void OpenGLRenderer::endFrame(const Window& window) {
        glfwSwapBuffers(&window.getWindowHandle());
    }

    void OpenGLRenderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer, const DescriptorSet* descriptorSet) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")

        currentRenderPass = static_cast<const OpenGLRenderPass*>(renderPass);
        const RenderPassSpecification& spec = currentRenderPass->getSpecification();

        glUseProgram(currentRenderPass->getOpenGLShaderHandle());

        if (descriptorSet) {
            static_cast<const OpenGLDescriptorSet*>(descriptorSet)->bind();
        }

        auto* fb = static_cast<const OpenGLFramebuffer*>(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->getOpenGLHandle());
        glViewport(0, 0, static_cast<int>(fb->getWidth()), static_cast<int>(fb->getHeight()));

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
            glDepthFunc(static_cast<GLint>(depthCompareOperator));
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
            glBlendEquation(static_cast<GLint>(blendingEquation));
        }
        if (srcBlendingFunction != spec.srcBlendingFunction || destBlendingFunction != spec.destBlendingFunction) {
            srcBlendingFunction = spec.srcBlendingFunction;
            destBlendingFunction = spec.destBlendingFunction;
            glBlendFunc(static_cast<GLint>(srcBlendingFunction), static_cast<GLint>(destBlendingFunction));
        }

        if (spec.clearColorAttachments) {
            const glm::vec4& clearColor = spec.clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (spec.clearDepthAttachment) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        if (spec.clearStencilBuffer) {
            glClear(GL_STENCIL_BUFFER_BIT);
        }

        if (spec.tesselationPatchSize != ~0 && spec.tesselationPatchSize != tessellationPatchSize) {
            tessellationPatchSize = spec.tesselationPatchSize;
            glPatchParameteri(GL_PATCH_VERTICES, tessellationPatchSize);
        }
    }

    void OpenGLRenderer::endRenderPass() {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")
        currentRenderPass = nullptr;
    }

    void OpenGLRenderer::clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")

        auto* fb = static_cast<const OpenGLFramebuffer*>(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fb->getOpenGLHandle());
        glViewport(0, 0, static_cast<int>(fb->getWidth()), static_cast<int>(fb->getHeight()));

        const RenderPassSpecification& spec = static_cast<const OpenGLRenderPass*>(renderPass)->getSpecification();

        if (spec.clearColorAttachments) {
            const glm::vec4& clearColor = spec.clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (spec.clearDepthAttachment) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        if (spec.clearStencilBuffer) {
            glClear(GL_STENCIL_BUFFER_BIT);
        }
    }

    void OpenGLRenderer::setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) {
        for (uint32_t i = 0; i < pushConstantsCount; i++) {
            const auto* pc = static_cast<const OpenGLPushConstants*>(pushConstants[i]);
            pc->upload(currentRenderPass->getOpenGLShaderHandle());
        }
    }

    void OpenGLRenderer::renderUnitQuad(const Material& material) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        quadVAO.bind();
        glDrawElements(GL_TRIANGLES, quadVAO.getIndexBuffer()->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRenderer::renderUnitCube(const Material& material) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        unitCubeVAO.bind();
        glDrawElements(GL_TRIANGLES, unitCubeVAO.getIndexBuffer()->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        auto* glVao = static_cast<const OpenGLVertexArrayObject*>(vao);

        glVao->bind();
        glDrawElementsInstancedBaseVertex(currentRenderPass->getDrawMode(), indexCount, GL_UNSIGNED_INT, (void*)(baseIndex * sizeof(uint32_t)), instanceCount, baseVertex);
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

//        environmentMapSphereToCube.bind();
//        environmentMapSphereToCube.setTexture2D(sphereMap, 0);
//        environmentMapSphereToCube.setImageCube(cubeMap, 1, ShaderStorageAccess::WriteOnly, 0);
//        environmentMapSphereToCube.dispatch(MAP_SIZE / 32, MAP_SIZE / 32, 6);
//        environmentMapSphereToCube.waitForMemoryBarrier({MemoryBarrierBit::All});
//
//        cubeMap.generateMipMaps();
//        TextureUtils::applyMipMapFiltering(MipMapFiltering::Trilinear, GL_TEXTURE_CUBE_MAP);

        uint32_t mipCount = Helpers::calculateMipCount(MAP_SIZE, MAP_SIZE);

        auto* prefilterMap = new OpenGLTextureCube(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Trilinear);
        prefilterMap->generateMipMaps();

//        environmentMapPrefilterMap.bind();
//        environmentMapPrefilterMap.setTextureCube(cubeMap, 0);

//        for (uint32_t i = 0, size = MAP_SIZE; i < mipCount; i++, size /= 2) {
//            uint32_t numGroups = glm::max(1u, size / 32);
//            float roughness = static_cast<float>(i) / static_cast<float>(mipCount - 1);
//            environmentMapPrefilterMap.setFloat("u_Roughness", roughness);
//            environmentMapPrefilterMap.setImageCube(*prefilterMap, 1, ShaderStorageAccess::WriteOnly, i);
//            environmentMapPrefilterMap.dispatch(numGroups, numGroups, 6);
//            environmentMapPrefilterMap.waitForMemoryBarrier({MemoryBarrierBit::All});
//        }

        auto* irradianceMap = new OpenGLTextureCube(TextureFormat::Float32A, 32, 32, MipMapFiltering::Bilinear);

//        environmentMapIrradianceMap.bind();
//        environmentMapIrradianceMap.setTextureCube(*prefilterMap, 0);
//        environmentMapIrradianceMap.setImageCube(*irradianceMap, 1, ShaderStorageAccess::WriteOnly);
//        environmentMapIrradianceMap.dispatch(irradianceMap->getWidth() / 2, irradianceMap->getWidth() / 2, 6);
//        environmentMapIrradianceMap.waitForMemoryBarrier({MemoryBarrierBit::All});

        return {irradianceMap, prefilterMap};
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
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif
    }

    void OpenGLRenderer::initImGui(Window& window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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
