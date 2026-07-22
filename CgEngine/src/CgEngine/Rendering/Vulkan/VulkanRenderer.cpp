#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

#define VMA_IMPLEMENTATION
#include "FileSystem.h"
#include "vk_mem_alloc.h"

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
        createVmaAllocator();
        descriptorAllocator.init(vkDevice);
        samplerManager.init(vkDevice);

        auto unitQuadVertexData = getUnitQuadVerticesAndIndices();

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

        brdfLUT = VulkanTexture2D(FileSystem::getAsEnginePath("ibl_brdf_lut.png"), false, TextureWrap::Clamp, MipMapFiltering::Bilinear);
    }

    void VulkanRenderer::shutdown() {
        descriptorAllocator.shutdown();
        samplerManager.shutdown();
    }

    void VulkanRenderer::setFramebufferResized() {

    }

    void VulkanRenderer::beginFrame(const Window& window) {

    }

    void VulkanRenderer::endFrame(const Window& window) {

    }

    void VulkanRenderer::beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {

    }

    void VulkanRenderer::beginSwapChainRenderPass() {

    }

    void VulkanRenderer::endRenderPass() {

    }

    void VulkanRenderer::beginDynamicRendering(const DynamicRenderingInfo &renderingInfo) {

    }

    void VulkanRenderer::endDynamicRendering() {

    }

    void VulkanRenderer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {

    }

    void VulkanRenderer::bindComputePipeline(const ComputePipeline* computePipeline) {

    }

    void VulkanRenderer::dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {

    }

    void VulkanRenderer::clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) {

    }

    void VulkanRenderer::bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) {

    }

    void VulkanRenderer::setPushConstants(const void* data, size_t size) {

    }

    void VulkanRenderer::transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) {

    }

    void VulkanRenderer::memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) {

    }

    void VulkanRenderer::memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment) {

    }

    void VulkanRenderer::renderUnitQuad() {

    }

    void VulkanRenderer::renderUnitCube() {

    }

    void VulkanRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) {

    }

    void VulkanRenderer::executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) {

    }

    void VulkanRenderer::drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) {

    }

    Texture2D* VulkanRenderer::getWhiteTexture() {
        return &whiteTexture;
    }

    Texture2D* VulkanRenderer::getBrdfLUTTexture() {
        return &brdfLUT;
    }

    TextureCube* VulkanRenderer::getBlackCubeTexture() {
        return nullptr;
    }

    std::pair<TextureCube*, TextureCube*> VulkanRenderer::createEnvironmentMap(const std::string& hdriPath) {
        return std::pair<TextureCube*, TextureCube*>();
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitQuadVertexInputLayout() {
        return quadVAO.getLayout();
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitCubeVertexInputLayout() {
        return unitCubeVAO.getLayout();
    }

    PipelineAttachmentInfo VulkanRenderer::getSwapChainAttachmentInfo() {
        return {};
    }

    void VulkanRenderer::beginImGuiFrame() {

    }

    void VulkanRenderer::renderImGuiFrame() {

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

    vk::PhysicalDevice VulkanRenderer::getVkPhysicalDevice() const {
        return vkPhysicalDevice;
    }

    vk::Device VulkanRenderer::getVkDevice() const {
        return vkDevice;
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
        CG_LOGGING_INFO("RENDERER: Name: {0}", std::string(props.deviceName));
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
            auto renderFinishedSemaphoreResult = vkDevice.createSemaphore(semaphoreCreateInfo);
            auto inFlightFenceResult = vkDevice.createFence(fenceCreateInfo);

            if (!imageAvailableSemaphoreResult.has_value() || !renderFinishedSemaphoreResult.has_value() || !inFlightFenceResult.has_value()) {
                CG_LOGGING_ERROR("VulkanRenderer: Failed to create synchronization objects for a frame!")
            }

            vkImageAvailableSemaphores.push_back(imageAvailableSemaphoreResult.value);
            vkRenderFinishedSemaphores.push_back(renderFinishedSemaphoreResult.value);
            vkInFlightFences.push_back(inFlightFenceResult.value);
        }
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

    vk::Bool32 VulkanRenderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }
}
