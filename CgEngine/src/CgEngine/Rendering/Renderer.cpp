#include <Application.h>
#include "Renderer.h"
#include "glad/glad.h"
#include "Asserts.h"
#include "FileSystem.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GraphicsObjectsFactory.h"

namespace CgEngine {
    void Renderer::init(Window& window) {
        backend = GraphicsObjectsFactory::createRendererBackend();
        backend->init(window);
    }

    void Renderer::shutdown() {
        backend->shutdown();
        delete backend;
    }

    void Renderer::beginFrame(const Window& window) {
        backend->beginFrame(window);
    }

    void Renderer::endFrame(const Window& window) {
        backend->endFrame(window);
    }

    void Renderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer, const DescriptorSet* descriptorSet) {
        backend->beginRenderPass(renderPass, framebuffer, descriptorSet);
    }

    void Renderer::endRenderPass() {
        backend->endRenderPass();
    }

    void Renderer::clearPass(const CgEngine::RenderPass* renderPass, const CgEngine::Framebuffer* framebuffer) {
        backend->clearPass(renderPass, framebuffer);
    }

    void Renderer::setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) {
        backend->setPushConstants(pushConstants, pushConstantsCount);
    }

    void Renderer::renderUnitQuad(const Material &material) {
        backend->renderUnitQuad(material);
    }

    void Renderer::renderUnitCube(const Material &material) {
        backend->renderUnitCube(material);
    }

//    void Renderer::renderLines(const std::vector<LineDrawInfo>& lines) {
//        if (lines.size() == 0) {
//            return;
//        }
//
//        linesVAO.bind();
//        auto& vertexBuffer = linesVAO.getVertexBuffers()[0];
//        auto vertices = std::vector<float>();
//
//        for (const auto& line: lines) {
//            vertices.push_back(line.from.x);
//            vertices.push_back(line.from.y);
//            vertices.push_back(line.from.z);
//
//            vertices.push_back(line.color.x);
//            vertices.push_back(line.color.y);
//            vertices.push_back(line.color.z);
//
//            vertices.push_back(line.to.x);
//            vertices.push_back(line.to.y);
//            vertices.push_back(line.to.z);
//
//            vertices.push_back(line.color.x);
//            vertices.push_back(line.color.y);
//            vertices.push_back(line.color.z);
//        }
//
//        vertexBuffer->setData(vertices.data(), vertices.size() * sizeof(float), VertexBufferUsage::Dynamic);
//        glDrawArrays(GL_LINES, 0, lines.size() * 2);
//    }

    void Renderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {
        backend->executeDrawCommand(vao, indexCount, baseIndex, baseVertex, instanceCount);
    }

//    void Renderer::executeCustomShaderDrawCommand(const VertexArrayObject& vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount, int tesselationPatchSize)  {
//        CG_ASSERT(currentRenderPass != nullptr, "There is no active RenderPass!")
//
//        vao.bind();
//        glDrawElementsInstancedBaseVertex(tesselationPatchSize == ~0 ? GL_TRIANGLES : GL_PATCHES, indexCount, GL_UNSIGNED_INT, (void*)(baseIndex * sizeof(uint32_t)), instanceCount, baseVertex);
//    }

    Texture2D* Renderer::getWhiteTexture() {
        return backend->getWhiteTexture();
    }

    Texture2D* Renderer::getBrdfLUTTexture() {
        return backend->getBrdfLUTTexture();
    }

    TextureCube* Renderer::getBlackCubeTexture() {
        return backend->getBlackCubeTexture();
    }

    std::pair<ResRef<TextureCube>, ResRef<TextureCube>> Renderer::createEnvironmentMap(const std::string &hdriPath) {
        auto& resourceManager = Application::get().getResourceManager();

        if (resourceManager.hasResource<TextureCube>(hdriPath + "-irradiance") && resourceManager.hasResource<TextureCube>(hdriPath + "-prefilter")) {
            auto irradianceMap = resourceManager.getResource<TextureCube>(hdriPath + "-irradiance");
            auto prefilterMap = resourceManager.getResource<TextureCube>(hdriPath + "-prefilter");
            return {irradianceMap, prefilterMap};
        }

        auto [irradianceMap, prefilterMap] =  backend->createEnvironmentMap(hdriPath);

        resourceManager.insertResource(hdriPath + "-irradiance", irradianceMap);
        resourceManager.insertResource(hdriPath + "-prefilter", prefilterMap);

        CG_LOGGING_DEBUG("Created Environment Map from: {0}", hdriPath)
        return {resourceManager.getResource<TextureCube>(hdriPath + "-irradiance"), resourceManager.getResource<TextureCube>(hdriPath + "-prefilter")};
    }

    void Renderer::beginImGuiFrame() {
        backend->beginImGuiFrame();
    }

    void Renderer::renderImGuiFrame() {
        backend->renderImGuiFrame();
    }
}
