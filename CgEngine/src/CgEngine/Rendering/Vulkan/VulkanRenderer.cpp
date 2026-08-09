#include "VulkanRenderer.h"
#include "VulkanHelpers.h"
#define VMA_IMPLEMENTATION
#include "Asserts.h"
#include "FileSystem.h"
#include "Logging.h"
#include "vk_mem_alloc.h"
#include "Rendering/Helpers.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "imgui.h"
#include "VulkanAttachment.h"
#include "VulkanComputePipeline.h"
#include "VulkanDescriptorSet.h"
#include "VulkanGraphicsPipeline.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace CgEngine {
    void VulkanRenderer::init(Window& window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ENABLE_VALIDATION_LAYERS = true;
        #endif

        if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport()) {
            CG_LOGGING_ERROR("VulkanRenderer: Error validation layers requested, but not available!")
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "CG_ENGINE";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        appInfo.pEngineName = "CG_ENGINE";
        appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = window.getRequiredVulkanExtensions();
        if (ENABLE_VALIDATION_LAYERS) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        createInfo.setPEnabledExtensionNames(extensions);

        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (ENABLE_VALIDATION_LAYERS) {
            createInfo.setPEnabledLayerNames(validationLayers);

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        auto instanceResult = vk::createInstance(createInfo);
        if (!instanceResult.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create Vulkan instance!")
        }
        vkInstance = instanceResult.value;
        vkDispatchLoaderDynamic.init(vkInstance, vkGetInstanceProcAddr);

        setupDebugMessenger();

        VkSurfaceKHR cSurface;
        if (glfwCreateWindowSurface(vkInstance, &window.getWindowHandle(), nullptr, &cSurface) != VK_SUCCESS) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create window surface!")
        }
        vkSurface = vk::SurfaceKHR(cSurface);

        pickPhysicalDevice();
        printDeviceInfo();
        createLogicalDevice();
        createSwapChain(window, VK_NULL_HANDLE);
        createSwapChainImageViews();
        createCommandPools();
        createGraphicsComputeCommandBuffers();
        createSyncObjects();
        createRenderFinishedSemaphores();
        createVmaAllocator();
        descriptorAllocator.init(vkDevice);
        samplerManager.init(vkDevice);

        auto unitQuadVertexData = getUnitQuadVerticesAndIndices(true);

        quadVAO = VulkanVertexArrayObject();
        auto* quadVertexBuffer = new VulkanVertexBuffer(std::get<0>(unitQuadVertexData).data(), std::get<0>(unitQuadVertexData).size() * sizeof(QuadVertex), VertexBufferUsage::Static);
        quadVertexBuffer->setLayout(std::get<2>(unitQuadVertexData));
        quadVAO.addVertexBuffer(quadVertexBuffer);
        quadVAO.setIndexBuffer(new VulkanIndexBuffer(std::get<1>(unitQuadVertexData).data(), std::get<1>(unitQuadVertexData).size(), IndexBufferDataType::UInt32));

        auto unitCubeVertexData = getUnitCubeVerticesAndIndices();

        unitCubeVAO = VulkanVertexArrayObject();
        auto* unitCubeVertexBuffer = new VulkanVertexBuffer(std::get<0>(unitCubeVertexData).data(), std::get<0>(unitCubeVertexData).size() * sizeof(float), VertexBufferUsage::Static);
        unitCubeVertexBuffer->setLayout(std::get<2>(unitCubeVertexData));
        unitCubeVAO.addVertexBuffer(unitCubeVertexBuffer);
        unitCubeVAO.setIndexBuffer(new VulkanIndexBuffer(std::get<1>(unitCubeVertexData).data(), std::get<1>(unitCubeVertexData).size()));

        constexpr uint32_t whiteTextureData = 0xffffffff;
        whiteTexture = VulkanTexture2D(TextureFormat::RGBA, 1, 1, TextureWrap::Clamp, &whiteTextureData, MipMapFiltering::Nearest);

        constexpr uint32_t blackCubeMapTextureData = 0xff000000;
        blackCubeTexture = VulkanTextureCube(TextureFormat::RGBA, 1, 1, &blackCubeMapTextureData, MipMapFiltering::Nearest);

        brdfLUT = VulkanTexture2D(FileSystem::getAsEnginePath("ibl_brdf_lut.png"), false, TextureWrap::Clamp, MipMapFiltering::Bilinear);

        DescriptorSetLayoutSpecification envMapComputeLayoutSpec{};
        envMapComputeLayoutSpec.texture2DAndAttachmentBindingPoints = {{0, DescriptorSetLayoutBindingUsage::Compute}};
        envMapComputeLayoutSpec.imageBindingPoints = {{1, DescriptorSetLayoutBindingUsage::Compute}};
        environmentMapComputeDescriptorSetLayout = VulkanDescriptorSetLayout(envMapComputeLayoutSpec);

        ComputePipelineSpecification sphereToCubeSpec{};
        sphereToCubeSpec.engineShaderName = "sphereToCube";
        sphereToCubeSpec.descriptorSetLayouts = {&environmentMapComputeDescriptorSetLayout};
        computeSphereToCube = VulkanComputePipeline(sphereToCubeSpec);

        ComputePipelineSpecification prefilterMapSpec{};
        prefilterMapSpec.engineShaderName = "prefilterMap";
        prefilterMapSpec.descriptorSetLayouts = {&environmentMapComputeDescriptorSetLayout};
        prefilterMapSpec.usesPushConstants = true;
        prefilterMapSpec.pushConstantsSize = sizeof(float);
        computePrefilterMap = VulkanComputePipeline(prefilterMapSpec);

        ComputePipelineSpecification irradianceMapSpec{};
        irradianceMapSpec.engineShaderName = "irradianceMap";
        irradianceMapSpec.descriptorSetLayouts = {&environmentMapComputeDescriptorSetLayout};
        computeIrradianceMap = VulkanComputePipeline(irradianceMapSpec);

        initImGui(window);
    }

    void VulkanRenderer::shutdown() {
        const auto idleResult = vkDevice.waitIdle();
        if (idleResult != vk::Result::eSuccess) {
            CG_LOGGING_ERROR("VulkanRenderer::shutdown: failed on waitIdle");
        }

        shutdownImGui();
        brdfLUT.deferredDestroyCurrentResources();
        whiteTexture.deferredDestroyCurrentResources();
        blackCubeTexture.deferredDestroyCurrentResources();
        unitCubeVAO.deferredDestroyCurrentResources();
        quadVAO.deferredDestroyCurrentResources();
        computeSphereToCube.deferredDestroyCurrentResources();
        computePrefilterMap.deferredDestroyCurrentResources();
        computeIrradianceMap.deferredDestroyCurrentResources();
        environmentMapComputeDescriptorSetLayout.deferredDestroyCurrentResources();

        for (auto& pending : pendingDestructions) {
            pending.destroy();
        }
        pendingDestructions.clear();

        descriptorAllocator.shutdown();
        samplerManager.shutdown();

        vmaDestroyAllocator(vmaAllocator);

        for (auto semaphore : vkRenderFinishedSemaphores) {
            vkDevice.destroySemaphore(semaphore);
        }
        vkRenderFinishedSemaphores.clear();

        for (auto semaphore : vkImageAvailableSemaphores) {
            vkDevice.destroySemaphore(semaphore);
        }
        for (auto fence : vkInFlightFences) {
            vkDevice.destroyFence(fence);
        }

        vkDevice.freeCommandBuffers(vkGraphicsComputeCommandPool, vkGraphicsComputeCommandBuffers);
        vkDevice.destroyCommandPool(vkTransientCommandPool);
        vkDevice.destroyCommandPool(vkGraphicsComputeCommandPool);

        for (const auto view : vkSwapChainImageViews) {
            vkDevice.destroyImageView(view);
        }

        vkDevice.destroySwapchainKHR(vkSwapChain);
        vkDevice.destroy();

        if (ENABLE_VALIDATION_LAYERS) {
            vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, vkDispatchLoaderDynamic);
        }
        vkInstance.destroySurfaceKHR(vkSurface);
        vkInstance.destroy();
    }

    void VulkanRenderer::setFramebufferResized() {
        framebufferResized = true;
    }

    bool VulkanRenderer::beginFrame(const Window& window) {
        if (window.getFramebufferWidth() <= 0 || window.getFramebufferHeight() <= 0) {
            return false;
        }

        vkDevice.waitForFences(1, &vkInFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);

        vk::Result result;
        do {
            result = vkDevice.acquireNextImageKHR(vkSwapChain, UINT64_MAX, vkImageAvailableSemaphores[currentFrameIndex], nullptr, &currentSwapChainImageIndex);
            if (result == vk::Result::eErrorOutOfDateKHR) {
                recreateSwapChain(window);
                framebufferResized = false;
            } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                CG_LOGGING_ERROR("VulkanRenderer: Failed to acquire swap chain image!")
                return false;
            }
        } while (result == vk::Result::eErrorOutOfDateKHR);

        if (vkImagesInFlight[currentSwapChainImageIndex]) {
            vkDevice.waitForFences(1, &vkImagesInFlight[currentSwapChainImageIndex], VK_TRUE, UINT64_MAX);
        }
        vkImagesInFlight[currentSwapChainImageIndex] = vkInFlightFences[currentFrameIndex];

        vkDevice.resetFences(1, &vkInFlightFences[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].reset();

        vk::CommandBufferBeginInfo beginInfo{};
        auto beginResult = vkGraphicsComputeCommandBuffers[currentFrameIndex].begin(beginInfo);
        CG_ASSERT(beginResult == vk::Result::eSuccess, "VulkanRenderer::beginFrame: Failed to begin command buffer!")

        vk::ImageMemoryBarrier2 imageBarrier{};
        imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
        imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        imageBarrier.setOldLayout(vk::ImageLayout::eUndefined);
        imageBarrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
        imageBarrier.setImage(vkSwapChainImages[currentSwapChainImageIndex]);
        imageBarrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        imageBarrier.subresourceRange.setBaseMipLevel(0);
        imageBarrier.subresourceRange.setLevelCount(1);
        imageBarrier.subresourceRange.setBaseArrayLayer(0);
        imageBarrier.subresourceRange.setLayerCount(1);
        imageBarrier.setSrcAccessMask({});
        imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);

        vk::DependencyInfo depInfo{};
        depInfo.setImageMemoryBarriers(imageBarrier);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].pipelineBarrier2(depInfo);

        frameIndex++;
        flushPendingDestructions();

        return true;
    }

    void VulkanRenderer::endFrame(const Window& window) {
        vk::ImageMemoryBarrier2 imageBarrier{};
        imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        imageBarrier.setDstStageMask({});
        imageBarrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
        imageBarrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
        imageBarrier.setImage(vkSwapChainImages[currentSwapChainImageIndex]);
        imageBarrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        imageBarrier.subresourceRange.setBaseMipLevel(0);
        imageBarrier.subresourceRange.setLevelCount(1);
        imageBarrier.subresourceRange.setBaseArrayLayer(0);
        imageBarrier.subresourceRange.setLayerCount(1);
        imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentRead);
        imageBarrier.setDstAccessMask({});

        vk::DependencyInfo depInfo{};
        depInfo.setImageMemoryBarriers(imageBarrier);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].pipelineBarrier2(depInfo);

        vkGraphicsComputeCommandBuffers[currentFrameIndex].end();

        vk::CommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.setCommandBuffer(vkGraphicsComputeCommandBuffers[currentFrameIndex]);

        vk::SemaphoreSubmitInfo waitSemaphoresSubmitInfo{};
        waitSemaphoresSubmitInfo.setSemaphore(vkImageAvailableSemaphores[currentFrameIndex]);
        waitSemaphoresSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        vk::SemaphoreSubmitInfo signalSemaphoresSubmitInfo{};
        signalSemaphoresSubmitInfo.setSemaphore(vkRenderFinishedSemaphores[currentSwapChainImageIndex]);
        signalSemaphoresSubmitInfo.setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        vk::SubmitInfo2 submitInfo{};
        submitInfo.setCommandBufferInfos(commandBufferInfo);
        submitInfo.setPWaitSemaphoreInfos(&waitSemaphoresSubmitInfo);
        submitInfo.setWaitSemaphoreInfoCount(1);
        submitInfo.setPSignalSemaphoreInfos(&signalSemaphoresSubmitInfo);
        submitInfo.setSignalSemaphoreInfoCount(1);

        auto submitResult = vkGraphicsComputeQueue.submit2(submitInfo, vkInFlightFences[currentFrameIndex]);
        CG_ASSERT(submitResult == vk::Result::eSuccess, "VulkanRenderer::endFrame: Failed to submit command buffer!")

        vk::PresentInfoKHR presentInfo{};
        presentInfo.setPWaitSemaphores(&vkRenderFinishedSemaphores[currentSwapChainImageIndex]);
        presentInfo.setWaitSemaphoreCount(1);
        presentInfo.setPSwapchains(&vkSwapChain);
        presentInfo.setSwapchainCount(1);
        presentInfo.setPImageIndices(&currentSwapChainImageIndex);

        auto presentResult = vkPresentQueue.presentKHR(&presentInfo);

        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
            recreateSwapChain(window);
        } else if (presentResult != vk::Result::eSuccess) {
            CG_LOGGING_ERROR("Failed to present swap chain image!")
        }

        currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRenderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        auto* rp = static_cast<const VulkanRenderPass*>(renderPass);
        auto* fb = static_cast<const VulkanFramebuffer*>(framebuffer);

        const auto& colorImageViews = fb->getVulkanColorAttachmentImageViews();
        const auto& colorFramebufferAttachments = fb->getColorFramebufferAttachments();

        std::vector<vk::RenderingAttachmentInfo> colorAttachmentInfos;
        colorAttachmentInfos.reserve(colorImageViews.size());

        constexpr VulkanAttachmentState neededColorAttachmentState = {
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .access = vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentRead,
            .stage = vk::PipelineStageFlagBits2::eColorAttachmentOutput
        };

        for (size_t i = 0; i < colorImageViews.size(); i++) {
            auto& attachmentInfo = colorAttachmentInfos.emplace_back();
            attachmentInfo.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
            attachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
            attachmentInfo.setLoadOp(rp->getColorAttachmentLoadOp());
            attachmentInfo.setClearValue(rp->getColorAttachmentClearValue());
            attachmentInfo.setImageView(colorImageViews[i]);

            barrierManager.requestAttachmentState(static_cast<VulkanAttachment*>(colorFramebufferAttachments[i].attachment), neededColorAttachmentState, colorFramebufferAttachments[i].allLayers, colorFramebufferAttachments[i].layer);
        }

        vk::RenderingInfo vkRenderingInfo{};

        vk::RenderingAttachmentInfo depthAttachmentInfo{};
        vk::RenderingAttachmentInfo stencilAttachmentInfo{};
        vk::ImageLayout depthStencilAttachmentLayout = vk::ImageLayout::eUndefined;

        if (fb->getVulkanDepthAttachmentImageView() != VK_NULL_HANDLE) {
            depthAttachmentInfo.setImageView(fb->getVulkanDepthAttachmentImageView());
            depthAttachmentInfo.setLoadOp(rp->getDepthStencilAttachmentLoadOp());
            depthAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
            depthAttachmentInfo.setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));
            depthStencilAttachmentLayout = vk::ImageLayout::eDepthAttachmentOptimal;

            vkRenderingInfo.setPDepthAttachment(&depthAttachmentInfo);
        }

        if (fb->getVulkanStencilAttachmentImageView() != VK_NULL_HANDLE) {
            stencilAttachmentInfo.setImageView(fb->getVulkanStencilAttachmentImageView());
            stencilAttachmentInfo.setLoadOp(rp->getDepthStencilAttachmentLoadOp());
            stencilAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
            depthAttachmentInfo.setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));
            depthStencilAttachmentLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            vkRenderingInfo.setPStencilAttachment(&stencilAttachmentInfo);
        }

        if (depthStencilAttachmentLayout != vk::ImageLayout::eUndefined) {
            VulkanAttachmentState neededDepthStencilAttachmentState = {
                .imageLayout = depthStencilAttachmentLayout,
                .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite | vk::AccessFlagBits2::eDepthStencilAttachmentRead,
                .stage = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests
            };
            FramebufferAttachment depthStencilFramebufferAttachment = fb->getDepthStencilFramebufferAttachment();
            barrierManager.requestAttachmentState(static_cast<VulkanAttachment*>(depthStencilFramebufferAttachment.attachment), neededDepthStencilAttachmentState, depthStencilFramebufferAttachment.allLayers, depthStencilFramebufferAttachment.layer);
        }

        depthAttachmentInfo.setImageLayout(depthStencilAttachmentLayout);
        stencilAttachmentInfo.setImageLayout(depthStencilAttachmentLayout);

        vkRenderingInfo.setColorAttachments(colorAttachmentInfos);
        vkRenderingInfo.setLayerCount(fb->getLayerCount());
        vkRenderingInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(fb->getWidth(), fb->getHeight())));

        barrierManager.flushBarriers(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].beginRendering(vkRenderingInfo);

        vk::Viewport viewport{};
        viewport.setX(0.0f);
        viewport.setY(0.0f);
        viewport.setWidth(static_cast<float>(fb->getWidth()));
        viewport.setHeight(static_cast<float>(fb->getHeight()));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setViewport(0, 1, &viewport);

        vk::Rect2D scissor{};
        scissor.setOffset(vk::Offset2D(0, 0));
        scissor.setExtent(vk::Extent2D(fb->getWidth(), fb->getHeight()));
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setScissor(0, 1, &scissor);
    }

    void VulkanRenderer::beginSwapChainRenderPass() {
        barrierManager.flushBarriers(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        beginSwapChainRenderPassInternal(true);
    }

    void VulkanRenderer::endRenderPass() {
        vkGraphicsComputeCommandBuffers[currentFrameIndex].endRendering();
    }

    void VulkanRenderer::beginDynamicRendering(const DynamicRenderingInfo &renderingInfo) {
        std::vector<vk::RenderingAttachmentInfo> colorAttachmentInfos;
        colorAttachmentInfos.reserve(renderingInfo.colorAttachments.size());

        uint32_t layers = 1;

        vk::RenderingInfo vkRenderingInfo{};

        for (auto attachment : renderingInfo.colorAttachments) {
            auto* vkAttachment = static_cast<VulkanAttachment*>(attachment.attachment);

            auto& attachmentInfo = colorAttachmentInfos.emplace_back();
            attachmentInfo.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
            attachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
            attachmentInfo.setLoadOp(renderingInfo.clearColorAttachments ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);

            if (renderingInfo.clearColorAttachments) {
                attachmentInfo.setClearValue(vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{renderingInfo.clearColor.r, renderingInfo.clearColor.g, renderingInfo.clearColor.b, renderingInfo.clearColor.a})));
            }

            VulkanAttachmentState neededState = {
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .access = vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentRead,
                .stage = vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };

            if (attachment.allLayers) {
                barrierManager.requestAttachmentState(vkAttachment, neededState, true);
                attachmentInfo.setImageView(vkAttachment->getVulkanImageView());
                layers = vkAttachment->getLayerCount();
            } else {
                barrierManager.requestAttachmentState(vkAttachment, neededState, false, attachment.layer);
                attachmentInfo.setImageView(vkAttachment->getVulkanLayerImageView(attachment.layer));
            }
        }

        vk::RenderingAttachmentInfo depthStencilAttachmentInfo{};

        if (renderingInfo.depthStencilAttachment.attachment != nullptr) {
            auto* vkAttachment = static_cast<VulkanAttachment*>(renderingInfo.depthStencilAttachment.attachment);
            CG_ASSERT(vkAttachment->getType() == AttachmentType::Depth || vkAttachment->getType() == AttachmentType::DepthStencil, "DepthStencil attachment must be of type Depth or DepthStencil")

            depthStencilAttachmentInfo.setImageLayout(vkAttachment->getType() == AttachmentType::Depth ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal);
            depthStencilAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);
            depthStencilAttachmentInfo.setLoadOp(renderingInfo.clearDepthStencilAttachment ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);

            if (renderingInfo.clearDepthStencilAttachment) {
                depthStencilAttachmentInfo.setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));
            }

            VulkanAttachmentState neededState = {
                .imageLayout = vkAttachment->getType() == AttachmentType::Depth ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal,
                .access = vk::AccessFlagBits2::eDepthStencilAttachmentWrite | vk::AccessFlagBits2::eDepthStencilAttachmentRead,
                .stage = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests
            };

            if (renderingInfo.depthStencilAttachment.allLayers) {
                barrierManager.requestAttachmentState(vkAttachment, neededState, true);
                depthStencilAttachmentInfo.setImageView(vkAttachment->getVulkanImageView());
                layers = vkAttachment->getLayerCount();
            } else {
                barrierManager.requestAttachmentState(vkAttachment, neededState, false, renderingInfo.depthStencilAttachment.layer);
                depthStencilAttachmentInfo.setImageView(vkAttachment->getVulkanLayerImageView(renderingInfo.depthStencilAttachment.layer));
            }

            if (vkAttachment->getType() == AttachmentType::Depth) {
                vkRenderingInfo.setPDepthAttachment(&depthStencilAttachmentInfo);
                vkRenderingInfo.setPStencilAttachment(nullptr);
            } else {
                vkRenderingInfo.setPDepthAttachment(&depthStencilAttachmentInfo);
                vkRenderingInfo.setPStencilAttachment(&depthStencilAttachmentInfo);
            }
        }

        vkRenderingInfo.setColorAttachments(colorAttachmentInfos);
        vkRenderingInfo.setLayerCount(layers);
        vkRenderingInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(renderingInfo.renderArea.x, renderingInfo.renderArea.y)));

        barrierManager.flushBarriers(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].beginRendering(vkRenderingInfo);

        vk::Viewport viewport{};
        viewport.setX(0.0f);
        viewport.setY(0.0f);
        viewport.setWidth(static_cast<float>(renderingInfo.renderArea.x));
        viewport.setHeight(static_cast<float>(renderingInfo.renderArea.y));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setViewport(0, 1, &viewport);


        vk::Rect2D scissor{};
        scissor.setOffset(vk::Offset2D(0, 0));
        scissor.setExtent(vk::Extent2D(renderingInfo.renderArea.x, renderingInfo.renderArea.y));
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setScissor(0, 1, &scissor);
    }

    void VulkanRenderer::endDynamicRendering() {
        vkGraphicsComputeCommandBuffers[currentFrameIndex].endRendering();
    }

    void VulkanRenderer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
        const auto* vkGraphicsPipeline = static_cast<const VulkanGraphicsPipeline*>(graphicsPipeline);
        currentPipelineLayout = vkGraphicsPipeline->getVulkanPipelineLayout();
        currentShaderStageFlags = vkGraphicsPipeline->getShaderStageFlags();
        currentPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        vkGraphicsComputeCommandBuffers[currentFrameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, vkGraphicsPipeline->getVulkanPipeline());
    }

    void VulkanRenderer::bindComputePipeline(const ComputePipeline* computePipeline) {
        const auto* vkComputePipeline = static_cast<const VulkanComputePipeline*>(computePipeline);
        currentPipelineLayout = vkComputePipeline->getVulkanPipelineLayout();
        currentShaderStageFlags = vk::ShaderStageFlagBits::eCompute;
        currentPipelineBindPoint = vk::PipelineBindPoint::eCompute;
        vkGraphicsComputeCommandBuffers[currentFrameIndex].bindPipeline(vk::PipelineBindPoint::eCompute, vkComputePipeline->getVulkanPipeline());
    }

    void VulkanRenderer::dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eCompute, "VulkanRenderer::dispatchCompute: Current pipeline bind point is not compute!")

        barrierManager.flushBarriers(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].dispatch(groupCountX, groupCountY, groupCountZ);
    }

    void VulkanRenderer::clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {
        beginRenderPass(renderPass, framebuffer);
        endRenderPass();
    }

    void VulkanRenderer::bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) {
        const auto* vkDescriptorSet = static_cast<const VulkanDescriptorSet*>(descriptorSet);
        auto dynamicOffsets = vkDescriptorSet->getDynamicBufferOffsets();
        vk::DescriptorSet set = vkDescriptorSet->getVulkanDescriptorSet();

        vkGraphicsComputeCommandBuffers[currentFrameIndex].bindDescriptorSets(currentPipelineBindPoint, currentPipelineLayout, setIndex, 1, &set, dynamicOffsets.size(), dynamicOffsets.data());
    }

    void VulkanRenderer::setPushConstants(const void* data, size_t size) {
        vkGraphicsComputeCommandBuffers[currentFrameIndex].pushConstants(currentPipelineLayout, currentShaderStageFlags, 0, static_cast<uint32_t>(size), data);
    }

    void VulkanRenderer::injectBarriersForDescriptorSet(const DescriptorSet *descriptorSet) {
        const auto* vkDescriptorSet = static_cast<const VulkanDescriptorSet*>(descriptorSet);

        const auto& attachmentTextureBindings = vkDescriptorSet->getAttachmentTextureBindings();

        for (auto& binding : attachmentTextureBindings) {
            auto* vkAttachment = static_cast<VulkanAttachment*>(binding.attachment);
            DescriptorSetLayoutBindingUsage usage = vkDescriptorSet->getLayout()->getDescriptorSetLayoutBindingUsageForBindingPoint(binding.bindingPoint);

            VulkanAttachmentState neededState{
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .access = vk::AccessFlagBits2::eShaderSampledRead,
                .stage = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanPipelineStageFlags(usage)
            };

            barrierManager.requestAttachmentState(vkAttachment, neededState, binding.allLayers, binding.layer);
        }

        const auto& attachmentImageBindings = vkDescriptorSet->getAttachmentImageBindings();

        for (auto& binding : attachmentImageBindings) {
            auto* vkAttachment = static_cast<VulkanAttachment*>(binding.attachment);
            DescriptorSetLayoutBindingUsage usage = vkDescriptorSet->getLayout()->getDescriptorSetLayoutBindingUsageForBindingPoint(binding.bindingPoint);

            vk::AccessFlags2 access{};

            if (binding.access == ShaderImageAccess::ReadOnly || binding.access == ShaderImageAccess::ReadWrite) {
                access |= vk::AccessFlagBits2::eShaderStorageRead;
            }
            if (binding.access == ShaderImageAccess::WriteOnly || binding.access == ShaderImageAccess::ReadWrite) {
                access |= vk::AccessFlagBits2::eShaderStorageWrite;
            }

            VulkanAttachmentState neededState{
                .imageLayout = vk::ImageLayout::eGeneral,
                .access = access,
                .stage = VulkanHelpers::descriptorSetLayoutBindingUsageToVulkanPipelineStageFlags(usage)
            };

            barrierManager.requestAttachmentState(vkAttachment, neededState, binding.allLayers, binding.layer);
        }
    }

    void VulkanRenderer::transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) {

    }

    void VulkanRenderer::memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) {

    }

    void VulkanRenderer::memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment) {

    }

    void VulkanRenderer::renderUnitQuad() {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eGraphics, "VulkanRenderer::renderUnitQuad: Current pipeline bind point is not graphics!")

        quadVAO.bind(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].drawIndexed(quadVAO.getIndexBuffer()->getIndexCount(), 1, 0, 0, 0);
    }

    void VulkanRenderer::renderUnitCube() {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eGraphics, "VulkanRenderer::renderUnitCube: Current pipeline bind point is not graphics!")

        unitCubeVAO.bind(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].drawIndexed(unitCubeVAO.getIndexBuffer()->getIndexCount(), 1, 0, 0, 0);
    }

    void VulkanRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eGraphics, "VulkanRenderer::executeDrawCommand: Current pipeline bind point is not graphics!")

        const auto* vkVao = static_cast<const VulkanVertexArrayObject*>(vao);
        vkVao->bind(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].drawIndexed(indexCount, instanceCount, baseIndex, baseVertex, 0);
    }

    void VulkanRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eGraphics, "VulkanRenderer::executeDrawCommand: Current pipeline bind point is not graphics!")

        const auto* vkVao = static_cast<const VulkanVertexArrayObject*>(vao);
        vkVao->bind(vkGraphicsComputeCommandBuffers[currentFrameIndex]);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].drawIndexed(indexCount, 1, baseIndex, baseVertex, 0);
    }

    void VulkanRenderer::drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) {
        CG_ASSERT(currentPipelineBindPoint == vk::PipelineBindPoint::eGraphics, "VulkanRenderer::drawArrays: Current pipeline bind point is not graphics!")

    }

    Texture2D* VulkanRenderer::getWhiteTexture() {
        return &whiteTexture;
    }

    Texture2D* VulkanRenderer::getBrdfLUTTexture() {
        return &brdfLUT;
    }

    TextureCube* VulkanRenderer::getBlackCubeTexture() {
        return &blackCubeTexture;
    }

    std::pair<TextureCube*, TextureCube*> VulkanRenderer::createEnvironmentMap(const std::string& hdriPath) {
        CG_LOGGING_DEBUG("Creating Environment Map from: {0}", hdriPath)

        constexpr uint32_t MAP_SIZE = 1024;
        constexpr vk::PipelineStageFlags2 computeStage = vk::PipelineStageFlagBits2::eComputeShader;

        VulkanTexture2D sphereMap(FileSystem::getAsGamePath(hdriPath), false);

        // Allocates a one-off descriptor set binding (0: sampled source, 1: storage image target mip), dispatches
        // the compute shader once and transitions the target mip back to shader-read-only for the next pass to sample.
        auto dispatchEnvMapCompute = [&](vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::ImageView sampledView, vk::Sampler sampledSampler, VulkanTextureCube* targetCube, uint32_t targetMip, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ, const void* pushConstantData, uint32_t pushConstantSize) {
            vk::DescriptorSet descriptorSet = descriptorAllocator.allocateDescriptorSet(environmentMapComputeDescriptorSetLayout.getDescriptorSetLayout());
            vk::ImageView storageView = targetCube->createStorageImageView(targetMip);

            vk::DescriptorImageInfo sampledInfo{};
            sampledInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            sampledInfo.imageView = sampledView;
            sampledInfo.sampler = sampledSampler;

            vk::DescriptorImageInfo storageInfo{};
            storageInfo.imageLayout = vk::ImageLayout::eGeneral;
            storageInfo.imageView = storageView;

            vk::WriteDescriptorSet sampledWrite{};
            sampledWrite.setDstSet(descriptorSet);
            sampledWrite.setDstArrayElement(0);
            sampledWrite.setDstBinding(0);
            sampledWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            sampledWrite.setDescriptorCount(1);
            sampledWrite.setPImageInfo(&sampledInfo);

            vk::WriteDescriptorSet storageWrite{};
            storageWrite.setDstSet(descriptorSet);
            storageWrite.setDstArrayElement(0);
            storageWrite.setDstBinding(1);
            storageWrite.setDescriptorType(vk::DescriptorType::eStorageImage);
            storageWrite.setDescriptorCount(1);
            storageWrite.setPImageInfo(&storageInfo);

            std::vector<vk::WriteDescriptorSet> writes = {sampledWrite, storageWrite};
            vkDevice.updateDescriptorSets(writes, {});

            executeImmediateCommand([&](const vk::CommandBuffer commandBuffer) {
                targetCube->recordLayoutTransition(commandBuffer, targetMip, 1, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral, computeStage, vk::AccessFlagBits2::eShaderSampledRead, computeStage, vk::AccessFlagBits2::eShaderStorageWrite);

                commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
                if (pushConstantData != nullptr) {
                    commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, pushConstantSize, pushConstantData);
                }
                commandBuffer.dispatch(groupsX, groupsY, groupsZ);

                targetCube->recordLayoutTransition(commandBuffer, targetMip, 1, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, computeStage, vk::AccessFlagBits2::eShaderStorageWrite, computeStage, vk::AccessFlagBits2::eShaderSampledRead);
            });

            vkDevice.destroyImageView(storageView);
        };

        // Equirectangular HDRI -> cube map (mip 0), then the regular mip chain is blitted from it.
        VulkanTextureCube cubeMap(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Trilinear);
        dispatchEnvMapCompute(computeSphereToCube.getVulkanPipeline(), computeSphereToCube.getVulkanPipelineLayout(), sphereMap.getVulkanImageView(), sphereMap.getVulkanSampler(), &cubeMap, 0, MAP_SIZE / 32, MAP_SIZE / 32, 6, nullptr, 0);
        cubeMap.generateMipMaps();

        // Prefiltered (roughness-mapped) mip chain, sampled from the mipped cubeMap with increasing roughness per mip.
        auto* prefilterMap = new VulkanTextureCube(TextureFormat::Float32A, MAP_SIZE, MAP_SIZE, MipMapFiltering::Trilinear);
        uint32_t mipCount = Helpers::calculateMipCount(MAP_SIZE, MAP_SIZE);
        for (uint32_t i = 0, size = MAP_SIZE; i < mipCount; i++, size /= 2) {
            uint32_t numGroups = glm::max(1u, size / 32);
            float roughness = mipCount > 1 ? static_cast<float>(i) / static_cast<float>(mipCount - 1) : 0.0f;

            dispatchEnvMapCompute(computePrefilterMap.getVulkanPipeline(), computePrefilterMap.getVulkanPipelineLayout(), cubeMap.getVulkanImageView(), cubeMap.getVulkanSampler(), prefilterMap, i, numGroups, numGroups, 6, &roughness, sizeof(float));
        }

        // Diffuse irradiance map, convolved from the prefiltered map.
        auto* irradianceMap = new VulkanTextureCube(TextureFormat::Float32A, 32, 32, MipMapFiltering::Bilinear);
        uint32_t irradianceGroups = irradianceMap->getWidth() / 2;
        dispatchEnvMapCompute(computeIrradianceMap.getVulkanPipeline(), computeIrradianceMap.getVulkanPipelineLayout(), prefilterMap->getVulkanImageView(), prefilterMap->getVulkanSampler(), irradianceMap, 0, irradianceGroups, irradianceGroups, 6, nullptr, 0);

        return {irradianceMap, prefilterMap};
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitQuadVertexInputLayout() {
        return quadVAO.getLayout();
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitCubeVertexInputLayout() {
        return unitCubeVAO.getLayout();
    }

    PipelineAttachmentInfo VulkanRenderer::getSwapChainAttachmentInfo() {
        PipelineAttachmentInfo info{};
        info.colorAttachments = {VulkanHelpers::vkColorFormatToAttachmentType(vkSwapChainImageFormat)};
        info.hasDepthStencilAttachment = false;

        return info;
    }

    void VulkanRenderer::beginImGuiFrame() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        #endif
    }

    void VulkanRenderer::renderImGuiFrame() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            beginSwapChainRenderPassInternal(false);

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkGraphicsComputeCommandBuffers[currentFrameIndex]);
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            endRenderPass();
        #endif
    }

    uint64_t VulkanRenderer::getFrameIndex() {
        return frameIndex;
    }

    void VulkanRenderer::executeImmediateCommand(const std::function<void(const vk::CommandBuffer&)> &lambda) const {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
        allocInfo.setCommandPool(vkTransientCommandPool);
        allocInfo.setCommandBufferCount(1);

        auto commandBufferResult = vkDevice.allocateCommandBuffers(allocInfo);
        CG_ASSERT(commandBufferResult.has_value(), "VulkanRenderer::executeImmediateCommand: Failed to allocate command buffer for immediate commands!")

        vk::CommandBuffer commandBuffer = commandBufferResult.value[0];

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        auto beginResult = commandBuffer.begin(beginInfo);
        CG_ASSERT(beginResult == vk::Result::eSuccess, "VulkanRenderer::executeImmediateCommand: Failed to begin command buffer for immediate commands!")

        lambda(commandBuffer);

        auto endResult = commandBuffer.end();
        CG_ASSERT(endResult == vk::Result::eSuccess, "VulkanRenderer::executeImmediateCommand: Failed to end command buffer for single time commands!")

        vk::CommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.setCommandBuffer(commandBuffer);

        vk::SubmitInfo2 submitInfo{};
        submitInfo.setCommandBufferInfos(commandBufferInfo);

        vk::FenceCreateInfo fenceInfo{};
        auto fenceResult = vkDevice.createFence(fenceInfo);
        CG_ASSERT(fenceResult.has_value(), "VulkanRenderer::executeImmediateCommand: Failed to create fence for immediate command buffer submission!")

        auto submitResult = vkGraphicsComputeQueue.submit2(submitInfo, fenceResult.value);
        CG_ASSERT(submitResult == vk::Result::eSuccess, "VulkanRenderer::executeImmediateCommand: Failed to submit command buffer for immediate commands!")

        auto waitFenceResult = vkDevice.waitForFences(fenceResult.value, VK_TRUE, UINT64_MAX);
        CG_ASSERT(waitFenceResult == vk::Result::eSuccess, "VulkanRenderer::executeImmediateCommand: Failed to wait for fence after submitting immediate command buffer!")

        vkDevice.destroyFence(fenceResult.value);
        vkDevice.freeCommandBuffers(vkTransientCommandPool, commandBuffer);
    }

    const uint32_t VulkanRenderer::getMaxFramesInFlight() const {
        return MAX_FRAMES_IN_FLIGHT;
    }

    const uint32_t VulkanRenderer::getCurrentFrameIndex() const {
        return currentFrameIndex;
    }

    void VulkanRenderer::deferDestruction(std::function<void()> destroyFn) {
        pendingDestructions.push_back({frameIndex + MAX_FRAMES_IN_FLIGHT, std::move(destroyFn)});
    }

    void VulkanRenderer::flushPendingDestructions() {
        std::erase_if(pendingDestructions, [this](const DeferredDestruction& pending) {
            if (pending.safeAtFrameIndex > frameIndex) {
                return false;
            }
            pending.destroy();
            return true;
        });
    }

    vk::PhysicalDevice VulkanRenderer::getVkPhysicalDevice() const {
        return vkPhysicalDevice;
    }

    vk::Device VulkanRenderer::getVkDevice() const {
        return vkDevice;
    }

    vk::CommandBuffer VulkanRenderer::getCurrentCommandBuffer() const {
        return vkGraphicsComputeCommandBuffers[currentFrameIndex];
    }

    VmaAllocator VulkanRenderer::getVmaAllocator() const {
        return vmaAllocator;
    }

    VulkanDescriptorAllocator& VulkanRenderer::getDescriptorAllocator() {
        return descriptorAllocator;
    }

    VulkanSamplerManager& VulkanRenderer::getSamplerManager() {
        return samplerManager;
    }

    bool VulkanRenderer::checkValidationLayerSupport() {
        auto result = vk::enumerateInstanceLayerProperties();
        if (!result.has_value()) {
            return false;
        }

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : result.value) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void VulkanRenderer::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanRenderer::setupDebugMessenger() {
        if (!ENABLE_VALIDATION_LAYERS) {
            return;
        }

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        auto result = vkInstance.createDebugUtilsMessengerEXT(createInfo, nullptr, vkDispatchLoaderDynamic);
        if (!result.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to set up debug messenger!")
        }
        debugMessenger = result.value;
    }

    void VulkanRenderer::pickPhysicalDevice() {
        auto devices = vkInstance.enumeratePhysicalDevices();
        if (!devices.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to enumerate physical devices!")
        }

        std::multimap<uint32_t, vk::PhysicalDevice> candidates;

        for (const auto& dev : devices.value) {
            uint32_t score = rateDeviceSuitability(dev, vkSurface);
            candidates.insert(std::make_pair(score, dev));
        }

        if (candidates.rbegin()->first > 0) {
            vkPhysicalDevice = candidates.rbegin()->second;
        } else {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to find suitable GPU!")
        }
    }

    void VulkanRenderer::printDeviceInfo() {
        vk::PhysicalDeviceProperties props = vkPhysicalDevice.getProperties();

        std::string vendorName;
        switch (props.vendorID) {
            case 0x10DE: vendorName = "NVIDIA"; break;
            case 0x1002: vendorName = "AMD"; break;
            case 0x8086: vendorName = "Intel"; break;
            case 0x13B5: vendorName = "ARM"; break;
            case 0x5143: vendorName = "Qualcomm"; break;
            default:     vendorName = "Unknown"; break;
        }

        CG_LOGGING_INFO("RENDERER: API: Vulkan");
        CG_LOGGING_INFO("RENDERER: Vendor ID: {0}", props.vendorID);
        CG_LOGGING_INFO("RENDERER: Vendor: {0}", vendorName);
        CG_LOGGING_INFO("RENDERER: Name: {0}", std::string(props.deviceName.data()));
        CG_LOGGING_INFO("RENDERER: Device ID: {0}", props.deviceID);

        uint32_t major = VK_VERSION_MAJOR(props.apiVersion);
        uint32_t minor = VK_VERSION_MINOR(props.apiVersion);
        uint32_t patch = VK_VERSION_PATCH(props.apiVersion);

        CG_LOGGING_INFO("RENDERER: API Version: {0}.{1}.{2}", major, minor, patch);

        auto limits = props.limits;

        CG_LOGGING_INFO("RENDERER: Max UBO size: {0}", limits.maxUniformBufferRange);
        CG_LOGGING_INFO("RENDERER: Max SSBO size: {0}", limits.maxStorageBufferRange);
        CG_LOGGING_INFO("RENDERER: Max bound descriptor sets: {0}", limits.maxBoundDescriptorSets);
    }

    uint32_t VulkanRenderer::rateDeviceSuitability(vk::PhysicalDevice dev, vk::SurfaceKHR surf) {
        auto deviceProperties = dev.getProperties();
        auto deviceFeatures = dev.getFeatures();

        uint32_t score = 0;

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        score += deviceProperties.limits.maxImageDimension2D;

        VulkanQueueFamilyIndices indices = findQueueFamilies(dev);
        bool extensionsSupported = checkDeviceExtensionSupport(dev);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(dev, surf);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        if (!indices.isComplete() || !deviceFeatures.geometryShader || !extensionsSupported || !swapChainAdequate) {
            return 0;
        }

        return score;
    }

    VulkanQueueFamilyIndices VulkanRenderer::findQueueFamilies(vk::PhysicalDevice device) {
        VulkanQueueFamilyIndices indices;

        auto queueFamilies = device.getQueueFamilyProperties();

        int i = 0;
        for (const auto& queueFamily: queueFamilies) {
            if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
                indices.graphicsComputeFamily = i;
            }

            auto presentSupport = device.getSurfaceSupportKHR(i, vkSurface);
            if (presentSupport.value) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    bool VulkanRenderer::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
        auto availableExtensions = device.enumerateDeviceExtensionProperties();
        if (!availableExtensions.has_value()) {
            return false;
        }

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions.value) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();

    }

    VulkanSwapChainSupportDetails VulkanRenderer::querySwapChainSupport(vk::PhysicalDevice dev, vk::SurfaceKHR surf) {
        VulkanSwapChainSupportDetails details;

        details.capabilities = dev.getSurfaceCapabilitiesKHR(surf).value;
        details.formats = dev.getSurfaceFormatsKHR(surf).value;
        details.presentModes = dev.getSurfacePresentModesKHR(surf).value;

        return details;
    }

    void VulkanRenderer::createLogicalDevice() {
        VulkanQueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsComputeFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.tessellationShader = VK_TRUE;
        deviceFeatures.shaderInt64 = VK_TRUE;

        vk::PhysicalDeviceVulkan13Features vk13PhysicalDeviceFeatures{};
        vk13PhysicalDeviceFeatures.setDynamicRendering(VK_TRUE);
        vk13PhysicalDeviceFeatures.setSynchronization2(VK_TRUE);

        vk::DeviceCreateInfo createInfo{};
        createInfo.setQueueCreateInfos(queueCreateInfos);
        createInfo.setPEnabledFeatures(&deviceFeatures);
        createInfo.setPEnabledExtensionNames(deviceExtensions);
        createInfo.setPNext(&vk13PhysicalDeviceFeatures);

        auto device = vkPhysicalDevice.createDevice(createInfo);
        if (!device.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create logical device!")
        }

        vkDevice = device.value;

        vkGraphicsComputeQueue = vkDevice.getQueue(indices.graphicsComputeFamily.value(), 0);
        vkPresentQueue = vkDevice.getQueue(indices.presentFamily.value(), 0);

        vkQueueIndices.graphicsComputeFamily = indices.graphicsComputeFamily.value();
        vkQueueIndices.presentFamily = indices.presentFamily.value();
    }

    void VulkanRenderer::createSwapChain(const Window& window, vk::SwapchainKHR oldSwapChain) {
        VulkanSwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(vkPhysicalDevice, vkSurface);
        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes, window);
        vk::Extent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities, window);

        uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
        if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount) {
            imageCount = swapChainSupportDetails.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = vkSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        uint32_t queueFamilyIndices[] = {vkQueueIndices.presentFamily, vkQueueIndices.graphicsComputeFamily};
        if (vkQueueIndices.graphicsComputeFamily != vkQueueIndices.presentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain;

        auto swapChainResult = vkDevice.createSwapchainKHR(createInfo);
        if (!swapChainResult.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create swap chain!")
        }
        vkSwapChain = swapChainResult.value;

        vkSwapChainImages = vkDevice.getSwapchainImagesKHR(vkSwapChain).value;
        vkSwapChainImageFormat = surfaceFormat.format;
        vkSwapChainExtent = extent;
    }

    vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if ((availableFormat.format == vk::Format::eB8G8R8A8Srgb || availableFormat.format == vk::Format::eR8G8B8A8Srgb) && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, const Window& window) {
        if (!window.isVsync()) {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                    return availablePresentMode;
                }
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width = window.getFramebufferWidth();
        int height = window.getFramebufferHeight();

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    void VulkanRenderer::createSwapChainImageViews() {
        vkSwapChainImageViews.resize(vkSwapChainImages.size());
        for (size_t i = 0; i < vkSwapChainImages.size(); i++) {
            vkSwapChainImageViews[i] = VulkanHelpers::createImageView2D(vkSwapChainImages[i], vkSwapChainImageFormat, 1, 1, vk::ImageAspectFlagBits::eColor);
        }
    }

    void VulkanRenderer::recreateSwapChain(const Window &window) {
        auto idleResult = vkDevice.waitIdle();
        if (idleResult != vk::Result::eSuccess) {
            CG_LOGGING_ERROR("SwapChain recreation failed on waitIdle()")
        }

        for (auto imageView : vkSwapChainImageViews) {
            vkDevice.destroyImageView(imageView);
        }
        vkSwapChainImageViews.clear();

        const vk::SwapchainKHR oldSwapChain = vkSwapChain;
        createSwapChain(window, oldSwapChain);

        if (oldSwapChain != VK_NULL_HANDLE) {
            vkDevice.destroySwapchainKHR(oldSwapChain);
        }
        createSwapChainImageViews();
        createRenderFinishedSemaphores();

        framebufferResized = false;
    }

    void VulkanRenderer::createCommandPools() {
        vk::CommandPoolCreateInfo graphicsComputePoolInfo{};
        graphicsComputePoolInfo.setQueueFamilyIndex(vkQueueIndices.graphicsComputeFamily);
        graphicsComputePoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        auto graphicsComputePool = vkDevice.createCommandPool(graphicsComputePoolInfo);
        if (!graphicsComputePool.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create graphics/compute command pool!")
        }
        vkGraphicsComputeCommandPool = graphicsComputePool.value;

        vk::CommandPoolCreateInfo transientPoolInfo{};
        transientPoolInfo.setQueueFamilyIndex(vkQueueIndices.graphicsComputeFamily);
        transientPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient);

        auto transientPool = vkDevice.createCommandPool(transientPoolInfo);
        if (!transientPool.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create graphics/compute command pool!")
        }
        vkTransientCommandPool = transientPool.value;
    }

    void VulkanRenderer::createGraphicsComputeCommandBuffers() {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.setCommandPool(vkGraphicsComputeCommandPool);
        commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
        commandBufferAllocateInfo.setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

        auto commandBuffersResult = vkDevice.allocateCommandBuffers(commandBufferAllocateInfo);
        if (!commandBuffersResult.has_value()) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to allocate graphics/compute command buffers!")
        }
        vkGraphicsComputeCommandBuffers = std::move(commandBuffersResult.value);
    }

    void VulkanRenderer::createSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};

        vk::FenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            auto imageAvailableSemaphoreResult = vkDevice.createSemaphore(semaphoreCreateInfo);
            auto inFlightFenceResult = vkDevice.createFence(fenceCreateInfo);

            if (!imageAvailableSemaphoreResult.has_value() || !inFlightFenceResult.has_value()) {
                CG_LOGGING_ERROR("VulkanRenderer: Failed to create synchronization objects for a frame!")
            }

            vkImageAvailableSemaphores.push_back(imageAvailableSemaphoreResult.value);
            vkInFlightFences.push_back(inFlightFenceResult.value);
        }
    }

    void VulkanRenderer::createRenderFinishedSemaphores() {
        for (auto semaphore : vkRenderFinishedSemaphores) {
            vkDevice.destroySemaphore(semaphore);
        }
        vkRenderFinishedSemaphores.clear();

        vk::SemaphoreCreateInfo semaphoreCreateInfo{};

        for (size_t i = 0; i < vkSwapChainImages.size(); i++) {
            auto renderFinishedSemaphoreResult = vkDevice.createSemaphore(semaphoreCreateInfo);
            if (!renderFinishedSemaphoreResult.has_value()) {
                CG_LOGGING_ERROR("VulkanRenderer: Failed to create render finished semaphore for a swap chain image!")
            }

            vkRenderFinishedSemaphores.push_back(renderFinishedSemaphoreResult.value);
        }

        vkImagesInFlight.assign(vkSwapChainImages.size(), VK_NULL_HANDLE);
    }

    void VulkanRenderer::createVmaAllocator() {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = vkPhysicalDevice;
        allocatorInfo.device = vkDevice;
        allocatorInfo.instance = vkInstance;
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

        auto result = vmaCreateAllocator(&allocatorInfo, &vmaAllocator);

        if (result != VK_SUCCESS) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create VMA allocator!")
        }
    }

    void VulkanRenderer::initImGui(Window &window) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            IMGUI_CHECKVERSION();

            vk::DescriptorPoolSize poolSizes[] = {
                { vk::DescriptorType::eSampler, 1000 },
                { vk::DescriptorType::eCombinedImageSampler, 1000 },
                { vk::DescriptorType::eSampledImage, 1000 },
                { vk::DescriptorType::eStorageImage, 1000 },
                { vk::DescriptorType::eUniformTexelBuffer, 1000 },
                { vk::DescriptorType::eStorageTexelBuffer, 1000 },
                { vk::DescriptorType::eUniformBuffer, 1000 },
                { vk::DescriptorType::eStorageBuffer, 1000 },
                { vk::DescriptorType::eUniformBufferDynamic, 1000 },
                { vk::DescriptorType::eStorageBufferDynamic, 1000 },
                { vk::DescriptorType::eInputAttachment, 1000 },
            };

            vk::DescriptorPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.setMaxSets(1000 * std::size(poolSizes));
            poolCreateInfo.setPoolSizes(poolSizes);
            poolCreateInfo.setPoolSizeCount(std::size(poolSizes));
            poolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

            auto poolCreateResult = vkDevice.createDescriptorPool(poolCreateInfo);

            if (!poolCreateResult.has_value()) {
                CG_LOGGING_ERROR("VulkanRenderer: Failed to create descriptor pool!")
            }
            imguiDescriptorPool = poolCreateResult.value;

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            glm::vec2 contentScale = window.getContentScale();

            ImGui::StyleColorsDark();
            ImGui::GetStyle().ScaleAllSizes(glm::max(contentScale.x, contentScale.y));

            ImGui_ImplGlfw_InitForVulkan(&window.getWindowHandle(), true);

            vk::PipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
            pipelineRenderingCreateInfo.setColorAttachmentCount(1);
            pipelineRenderingCreateInfo.setPColorAttachmentFormats(&vkSwapChainImageFormat);

            ImGui_ImplVulkan_InitInfo initInfo{};
            initInfo.ApiVersion = VK_API_VERSION_1_3;
            initInfo.Instance = vkInstance;
            initInfo.PhysicalDevice = vkPhysicalDevice;
            initInfo.Device = vkDevice;
            initInfo.QueueFamily = vkQueueIndices.graphicsComputeFamily;
            initInfo.Queue = vkGraphicsComputeQueue;
            initInfo.DescriptorPool = imguiDescriptorPool;
            initInfo.MinImageCount = vkSwapChainImages.size();
            initInfo.ImageCount = vkSwapChainImages.size();
            initInfo.UseDynamicRendering = true;
            initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

            ImGui_ImplVulkan_Init(&initInfo);
        #endif
    }

    void VulkanRenderer::shutdownImGui() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            vkDevice.destroyDescriptorPool(imguiDescriptorPool);
        #endif
    }

    void VulkanRenderer::beginSwapChainRenderPassInternal(bool clear) {
        vk::RenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.setImageView(vkSwapChainImageViews[currentSwapChainImageIndex]);
        colorAttachmentInfo.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
        colorAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);

        if (clear) {
            colorAttachmentInfo.setClearValue(vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})));
            colorAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
        } else {
            colorAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eLoad);
        }

        vk::RenderingInfo renderingInfo{};
        renderingInfo.setColorAttachmentCount(1);
        renderingInfo.setPColorAttachments(&colorAttachmentInfo);
        renderingInfo.setLayerCount(1);
        renderingInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vkSwapChainExtent));

        vkGraphicsComputeCommandBuffers[currentFrameIndex].beginRendering(renderingInfo);

        vk::Viewport viewport{};
        viewport.setX(0.0f);
        viewport.setY(0.0f);
        viewport.setWidth(static_cast<float>(vkSwapChainExtent.width));
        viewport.setHeight(static_cast<float>(vkSwapChainExtent.height));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setViewport(0, 1, &viewport);


        vk::Rect2D scissor{};
        scissor.setOffset(vk::Offset2D(0, 0));
        scissor.setExtent(vk::Extent2D(vkSwapChainExtent.width, vkSwapChainExtent.height));
        vkGraphicsComputeCommandBuffers[currentFrameIndex].setScissor(0, 1, &scissor);
    }

    vk::Bool32 VulkanRenderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }
}
