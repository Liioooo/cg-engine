#include <Application.h>
#include "Renderer.h"
#include "glad/glad.h"
#include "Asserts.h"
#include "FileSystem.h"

namespace CgEngine {
    void Renderer::init() {
        const char* rendererName = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* rendererVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char* rendererVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        CG_LOGGING_INFO("RENDERER: Vendor: {0}", rendererVendor)
        CG_LOGGING_INFO("RENDERER: Name: {0}", rendererName)
        CG_LOGGING_INFO("RENDERER: Version: {0}", rendererVersion)

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

        quadVAO = new VertexArrayObject();
        auto* quadVertexBuffer = new VertexBuffer(quadVertexData, 4 * sizeof(QuadVertex));
        quadVertexBuffer->setLayout({{ShaderDataType::Float3, true}, {ShaderDataType::Float2, true}});
        quadVAO->addVertexBuffer(quadVertexBuffer);
        uint32_t quadIndices[6] = {0, 1, 2, 2, 3, 0 };
        quadVAO->setIndexBuffer(quadIndices, 6);

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

        unitCubeVAO = new VertexArrayObject();
        auto* unitCubeVertexBuffer = new VertexBuffer(unitCubeVertices, sizeof(unitCubeVertices));
        unitCubeVertexBuffer->setLayout({{ShaderDataType::Float3, false}});
        unitCubeVAO->addVertexBuffer(unitCubeVertexBuffer);
        unitCubeVAO->setIndexBuffer(unitCubeIndices, 36);

        linesVAO = new VertexArrayObject();
        auto* linesVertexBuffer = new VertexBuffer(0, VertexBufferUsage::Dynamic);
        linesVertexBuffer->setLayout({{ShaderDataType::Float3, false}, {ShaderDataType::Float3, false}});
        linesVAO->addVertexBuffer(linesVertexBuffer);

        auto* uiIndices = new uint32_t[maxUiIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < maxUiIndices; i += 6) {
            uiIndices[i + 0] = offset + 0;
            uiIndices[i + 1] = offset + 1;
            uiIndices[i + 2] = offset + 2;

            uiIndices[i + 3] = offset + 2;
            uiIndices[i + 4] = offset + 3;
            uiIndices[i + 5] = offset + 0;

            offset += 4;
        }

        uiCircleVAO = new VertexArrayObject();
        auto* uiCircleVertexBuffer = new VertexBuffer(0, VertexBufferUsage::Dynamic);
        uiCircleVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
        uiCircleVAO->addVertexBuffer(uiCircleVertexBuffer);
        uiCircleVAO->setIndexBuffer(uiIndices, maxUiIndices);

        uiRectVAO = new VertexArrayObject();
        auto* uiRectVertexBuffer = new VertexBuffer(0, VertexBufferUsage::Dynamic);
        uiCircleVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float2, false}, {ShaderDataType::Float, false}, {ShaderDataType::Float, false}});
        uiRectVAO->addVertexBuffer(uiCircleVertexBuffer);
        uiRectVAO->setIndexBuffer(uiIndices, maxUiIndices);

        uiTextVAO = new VertexArrayObject();
        auto* uiTextVertexBuffer = new VertexBuffer(0, VertexBufferUsage::Dynamic);
        uiTextVertexBuffer->setLayout({{ShaderDataType::Float4, false}, {ShaderDataType::Float4, false}, {ShaderDataType::Float, false}});
        uiTextVAO->addVertexBuffer(uiTextVertexBuffer);
        uiTextVAO->setIndexBuffer(uiIndices, maxUiIndices);

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
        glDepthFunc(static_cast<GLint>(depthCompareOperator));

        useBlending = false;
        glDisable(GL_BLEND);
        blendingEquation = BlendingEquation::Add;
        glBlendEquation(static_cast<GLint>(blendingEquation));
        srcBlendingFunction = BlendingFunction::SrcAlpha;
        destBlendingFunction = BlendingFunction::OneMinusSrcAlpha;
        glBlendFunc(static_cast<GLint>(srcBlendingFunction), static_cast<GLint>(destBlendingFunction));

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        uint32_t whiteTextureData = 0xffffffff;
        whiteTexture = new Texture2D(TextureFormat::RGBA, 1, 1, TextureWrap::Clamp, &whiteTextureData, MipMapFiltering::Nearest);

        uint32_t blackCubeMapTextureData = 0xff000000;
        blackCubeTexture = new TextureCube(TextureFormat::RGBA, 1, 1, &blackCubeMapTextureData, MipMapFiltering::Nearest);

        brdfLUT = new Texture2D(FileSystem::getAsEnginePath("ibl_brdf_lut.png"), false, TextureWrap::Clamp, MipMapFiltering::Bilinear);

        transformsBuffer = new ShaderStorageBuffer();
        transformsBuffer->bind(0);

        environmentMapSphereToCube = new ComputeShader("sphereToCube");
        environmentMapPrefilterMap = new ComputeShader("prefilterMap");
        environmentMapIrradianceMap = new ComputeShader("irradianceMap");
    }

    void Renderer::shutdown() {
        delete whiteTexture;
        delete blackCubeTexture;
        delete brdfLUT;
        delete quadVAO;
        delete unitCubeVAO;
        delete linesVAO;
        delete transformsBuffer;

        delete environmentMapSphereToCube;
        delete environmentMapPrefilterMap;
        delete environmentMapIrradianceMap;
    }

    void Renderer::beginRenderPass(RenderPass& renderPass, bool omitShaderBinding) {
        CG_ASSERT(currentRenderPass == nullptr, "There already is an active RenderPass!")
        CG_ASSERT(omitShaderBinding || renderPass.isReady(), "The RenderPass is not ready to use!")

        currentRenderPass = &renderPass;
        RenderPassSpecification& spec = renderPass.getSpecification();

        if (!omitShaderBinding) {
            spec.shader.bind();
        }

        spec.framebuffer->bind();

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

        if (spec.clearColorBuffer) {
            const glm::vec4& clearColor = spec.framebuffer->getSpecification().clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        if (spec.clearDepthBuffer) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        if (spec.clearStencilBuffer) {
            glClear(GL_STENCIL_BUFFER_BIT);
        }
    }

    void Renderer::endRenderPass() {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        currentRenderPass = nullptr;
    }

    void Renderer::setFaceCulling(bool backFaceCulling, bool frontFaceCulling) {
        if (isBackFaceCulling != backFaceCulling || isFrontFaceCulling != frontFaceCulling) {
            isBackFaceCulling = backFaceCulling;
            isFrontFaceCulling = frontFaceCulling;
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
    }

    void Renderer::setBlending(bool enable, CgEngine::BlendingEquation blendingEq, CgEngine::BlendingFunction srcBlendingFn, CgEngine::BlendingFunction destBlendingFn) {
        if (useBlending != enable) {
            useBlending = enable;
            if (useBlending) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
        }
        if (blendingEquation != blendingEq) {
            blendingEquation = blendingEq;
            glBlendEquation(static_cast<GLint>(blendingEquation));
        }
        if (srcBlendingFunction != srcBlendingFn || destBlendingFunction != destBlendingFn) {
            srcBlendingFunction = srcBlendingFn;
            destBlendingFunction = destBlendingFn;
            glBlendFunc(static_cast<GLint>(srcBlendingFunction), static_cast<GLint>(destBlendingFunction));
        }
    }

    void Renderer::renderUnitQuad(const Material &material) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        material.uploadToShader(currentRenderPass->getSpecification().shader);

        quadVAO->bind();
        glDrawElements(GL_TRIANGLES, quadVAO->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    void Renderer::renderUnitCube(const Material &material) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        material.uploadToShader(currentRenderPass->getSpecification().shader);

        unitCubeVAO->bind();
        glDrawElements(GL_TRIANGLES, unitCubeVAO->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    void Renderer::renderLines(const std::vector<LineDrawInfo>& lines) {
        if (lines.size() == 0) {
            return;
        }

        linesVAO->bind();
        auto& vertexBuffer = linesVAO->getVertexBuffers()[0];
        auto vertices = std::vector<float>();

        for (const auto& line: lines) {
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

        vertexBuffer->setData(vertices.data(), vertices.size() * sizeof(float), VertexBufferUsage::Dynamic);
        glDrawArrays(GL_LINES, 0, lines.size() * 2);
    }

    void Renderer::executeDrawCommand(const VertexArrayObject& vao, const Material& material, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, const std::vector<glm::mat4>& transforms, uint32_t instanceCount) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        material.uploadToShader(currentRenderPass->getSpecification().shader);
        vao.bind();
        transformsBuffer->setData(transforms.data(), transforms.size() * sizeof(glm::mat4));
        glDrawElementsInstancedBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(baseIndex * sizeof(uint32_t)), instanceCount, baseVertex);
    }

    void Renderer::executeCustomShaderDrawCommand(const CgEngine::VertexArrayObject& vao, const CgEngine::Material& material, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount, CustomShader& shader, bool needsMaterialUpload)  {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        if (needsMaterialUpload) {
            material.uploadToShader(shader);
        }
        vao.bind();
        glDrawElementsInstancedBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(baseIndex * sizeof(uint32_t)), instanceCount, baseVertex);
    }

    void Renderer::renderUiCircles(const std::vector<UiCircleVertex>& vertices, uint32_t indexCount) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        uiCircleVAO->bind();
        uiCircleVAO->getVertexBuffers()[0]->setData(vertices.data(), vertices.size() * sizeof(UiCircleVertex), VertexBufferUsage::Dynamic);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void Renderer::renderUiRects(const std::vector<UiRectVertex>& vertices, uint32_t indexCount) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        uiRectVAO->bind();
        uiRectVAO->getVertexBuffers()[0]->setData(vertices.data(), vertices.size() * sizeof(UiRectVertex), VertexBufferUsage::Dynamic);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void Renderer::renderUiText(const std::vector<UiTextVertex>& vertices, uint32_t indexCount) {
        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")

        uiTextVAO->bind();
        uiTextVAO->getVertexBuffers()[0]->setData(vertices.data(), vertices.size() * sizeof(UiTextVertex), VertexBufferUsage::Dynamic);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    Texture2D& Renderer::getWhiteTexture() {
        return *whiteTexture;
    }

    Texture2D &Renderer::getBrdfLUTTexture() {
        return *brdfLUT;
    }

    TextureCube& Renderer::getBlackCubeTexture() {
        return *blackCubeTexture;
    }

    std::pair<ResRef<TextureCube>, ResRef<TextureCube>> Renderer::createEnvironmentMap(const std::string &hdriPath) {
        auto& resourceManager = Application::get().getResourceManager();

        if (resourceManager.hasResource<TextureCube>(hdriPath + "-irradiance") && resourceManager.hasResource<TextureCube>(hdriPath + "-prefilter")) {
            auto irradianceMap = resourceManager.getResource<TextureCube>(hdriPath + "-irradiance");
            auto prefilterMap = resourceManager.getResource<TextureCube>(hdriPath + "-prefilter");
            return {irradianceMap, prefilterMap};
        }

        CG_LOGGING_DEBUG("Creating Environment Map from: {0}", hdriPath)

        const uint32_t MAP_SIZE = 1024;

        Texture2D sphereMap("assets/game/" + hdriPath, false);

        CG_ASSERT(sphereMap.isLoaded(), "HDRI could not be loaded!")

        TextureCube cubeMap(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Bilinear);

        environmentMapSphereToCube->bind();
        environmentMapSphereToCube->setTexture2D(sphereMap, 0);
        environmentMapSphereToCube->setImageCube(cubeMap, 1, ShaderStorageAccess::WriteOnly, 0);
        environmentMapSphereToCube->dispatch(MAP_SIZE / 32, MAP_SIZE / 32, 6);
        environmentMapSphereToCube->waitForMemoryBarrier();

        cubeMap.generateMipMaps();
        TextureUtils::applyMipMapFiltering(MipMapFiltering::Trilinear, GL_TEXTURE_CUBE_MAP);

        uint32_t mipCount = TextureUtils::calculateMipCount(MAP_SIZE, MAP_SIZE);

        auto* prefilterMap = new TextureCube(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Trilinear);
        prefilterMap->generateMipMaps();

        environmentMapPrefilterMap->bind();
        environmentMapPrefilterMap->setTextureCube(cubeMap, 0);

        for (uint32_t i = 0, size = MAP_SIZE; i < mipCount; i++, size /= 2) {
            uint32_t numGroups = glm::max(1u, size / 32);
            float roughness = static_cast<float>(i) / static_cast<float>(mipCount - 1);
            environmentMapPrefilterMap->setFloat("u_Roughness", roughness);
            environmentMapPrefilterMap->setImageCube(*prefilterMap, 1, ShaderStorageAccess::WriteOnly, i);
            environmentMapPrefilterMap->dispatch(numGroups, numGroups, 6);
            environmentMapPrefilterMap->waitForMemoryBarrier();
        }

        auto* irradianceMap = new TextureCube(TextureFormat::Float32A, 32, 32, MipMapFiltering::Bilinear);

        environmentMapIrradianceMap->bind();
        environmentMapIrradianceMap->setTextureCube(*prefilterMap, 0);
        environmentMapIrradianceMap->setImageCube(*irradianceMap, 1, ShaderStorageAccess::WriteOnly);
        environmentMapIrradianceMap->dispatch(irradianceMap->getWidth() / 2, irradianceMap->getWidth() / 2, 6);
        environmentMapIrradianceMap->waitForMemoryBarrier();

        resourceManager.insertResource(hdriPath + "-irradiance", irradianceMap);
        resourceManager.insertResource(hdriPath + "-prefilter", prefilterMap);

        CG_LOGGING_DEBUG("Created Environment Map from: {0}", hdriPath)
        return {resourceManager.getResource<TextureCube>(hdriPath + "-irradiance"), resourceManager.getResource<TextureCube>(hdriPath + "-prefilter")};
    }
}
