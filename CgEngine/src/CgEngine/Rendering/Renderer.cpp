#include <Application.h>
#include "Renderer.h"
#include "Asserts.h"
#include "GraphicsObjectsFactory.h"
#include "Rendering/OpenGL/OpenGLRenderer.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

namespace CgEngine {
    void Renderer::init(Window& window) {
        backend = GraphicsObjectsFactory::createRendererBackend();
        backend->init(window);
    }

    void Renderer::shutdown() {
        backend->shutdown();
        delete backend;
    }

    void Renderer::setFramebufferResized() {
        backend->setFramebufferResized();
    }

    bool Renderer::beginFrame(const Window& window) {
        return backend->beginFrame(window);
    }

    void Renderer::endFrame(const Window& window) {
        backend->endFrame(window);
    }

    void Renderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        backend->beginRenderPass(renderPass, framebuffer);
    }

    void Renderer::beginSwapChainRenderPass() {
        backend->beginSwapChainRenderPass();
    }

    void Renderer::endRenderPass() {
        backend->endRenderPass();
    }

    void Renderer::beginDynamicRendering(const DynamicRenderingInfo& renderingInfo) {
        backend->beginDynamicRendering(renderingInfo);
    }

    void Renderer::endDynamicRendering() {
        backend->endDynamicRendering();
    }

    void Renderer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
        backend->bindGraphicsPipeline(graphicsPipeline);
    }

    void Renderer::bindComputePipeline(const ComputePipeline* computePipeline) {
        backend->bindComputePipeline(computePipeline);
    }

    void Renderer::dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        backend->dispatchCompute(groupCountX, groupCountY, groupCountZ);
    }

    void Renderer::clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        backend->clearPass(renderPass, framebuffer);
    }

    void Renderer::bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) {
        backend->bindDescriptorSet(descriptorSet, setIndex);
    }

    void Renderer::setPushConstants(const void* data, size_t size) {
        backend->setPushConstants(data, size);
    }

    void Renderer::injectBarriersForDescriptorSet(const DescriptorSet *descriptorSet) {
        backend->injectBarriersForDescriptorSet(descriptorSet);
    }

    void Renderer::memoryBarrierForVertexBufferAfterCompute(VertexBuffer* vertexBuffer) {
        backend->memoryBarrierForVertexBufferAfterCompute(vertexBuffer);
    }

    void Renderer::renderUnitQuad() {
        backend->renderUnitQuad();
    }

    void Renderer::renderUnitCube() {
        backend->renderUnitCube();
    }

    void Renderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {
        backend->executeDrawCommand(vao, indexCount, baseIndex, baseVertex, instanceCount);
    }

    void Renderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) {
        backend->executeDrawCommand(vao, indexCount, baseIndex, baseVertex);
    }

    void Renderer::drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) {
        backend->drawArrays(vao, vertexCount);
    }

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

    const std::vector<VertexBufferLayout> Renderer::getUnitQuadVertexInputLayout() {
        return backend->getUnitQuadVertexInputLayout();
    }

    const std::vector<VertexBufferLayout> Renderer::getUnitCubeVertexInputLayout() {
        return backend->getUnitCubeVertexInputLayout();
    }

    const PipelineAttachmentInfo Renderer::getSwapChainAttachmentInfo() {
        return backend->getSwapChainAttachmentInfo();
    }

    void Renderer::beginImGuiFrame() {
        backend->beginImGuiFrame();
    }

    void Renderer::renderImGuiFrame() {
        backend->renderImGuiFrame();
    }

    uint64_t Renderer::getFrameIndex() {
        return backend->getFrameIndex();
    }

    OpenGLRenderer* Renderer::getOpenGLBackend() {
        CG_ASSERT(backend->getGraphicsAPI() == GraphicsAPI::OpenGL, "Renderer::getOpenGLBackend: Renderer backend is not OpenGL!")
        return static_cast<OpenGLRenderer*>(backend);
    }

    VulkanRenderer* Renderer::getVulkanBackend() {
        CG_ASSERT(backend->getGraphicsAPI() == GraphicsAPI::Vulkan, "Renderer::getVulkanBackend: Renderer backend is not Vulkan!")
        return static_cast<VulkanRenderer*>(backend);
    }
}
