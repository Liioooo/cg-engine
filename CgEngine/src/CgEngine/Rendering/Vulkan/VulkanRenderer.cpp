#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

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
        vkDispatchLoaderDynamic.init(static_cast<VkInstance>(vkInstance), vkGetInstanceProcAddr);

        setupDebugMessenger();

        VkSurfaceKHR cSurface;
        if (glfwCreateWindowSurface(vkInstance, &window.getWindowHandle(), nullptr, &cSurface) != VK_SUCCESS) {
            CG_LOGGING_ERROR("VulkanRenderer: Failed to create window surface!")
        }
        vkSurface = vk::SurfaceKHR(cSurface);

        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain(window, VK_NULL_HANDLE);
        createSwapChainImageViews();
    }

    void VulkanRenderer::shutdown() {

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
        return nullptr;
    }

    Texture2D* VulkanRenderer::getBrdfLUTTexture() {
        return nullptr;
    }

    TextureCube* VulkanRenderer::getBlackCubeTexture() {
        return nullptr;
    }

    std::pair<TextureCube*, TextureCube*> VulkanRenderer::createEnvironmentMap(const std::string& hdriPath) {
        return std::pair<TextureCube*, TextureCube*>();
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitQuadVertexInputLayout() {
        return std::vector<VertexBufferLayout>();
    }

    const std::vector<VertexBufferLayout> VulkanRenderer::getUnitCubeVertexInputLayout() {
        return std::vector<VertexBufferLayout>();
    }

    const RenderPass* VulkanRenderer::getSwapChainRenderPass() {
        return nullptr;
    }

    void VulkanRenderer::beginImGuiFrame() {

    }

    void VulkanRenderer::renderImGuiFrame() {

    }

    vk::PhysicalDevice VulkanRenderer::getVkPhysicalDevice() const {
        return vkPhysicalDevice;
    }

    vk::Device VulkanRenderer::getVkDevice() const {
        return vkDevice;
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

        vk::DeviceCreateInfo createInfo{};
        createInfo.setQueueCreateInfos(queueCreateInfos);
        createInfo.setPEnabledFeatures(&deviceFeatures);
        createInfo.setPEnabledExtensionNames(deviceExtensions);

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
        } else {
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
    }

    void VulkanRenderer::createSwapChainImageViews() {
        vkSwapChainImageViews.resize(vkSwapChainImages.size());
        for (size_t i = 0; i < vkSwapChainImages.size(); i++) {
            vkSwapChainImageViews[i] = VulkanHelpers::createImageView2D(vkSwapChainImages[i], vkSwapChainImageFormat, 1, 1, vk::ImageAspectFlagBits::eColor);
        }
    }

    vk::Bool32 VulkanRenderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }
}
