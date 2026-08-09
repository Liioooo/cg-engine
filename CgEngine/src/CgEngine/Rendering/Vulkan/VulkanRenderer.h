#pragma once

#include "Rendering/RendererBackendBase.h"
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include "VulkanBarrierManager.h"
#include "VulkanComputePipeline.h"
#include "VulkanDescriptorAllocator.h"
#include "VulkanDescriptorSet.h"
#include "VulkanSamplerManager.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureCube.h"
#include "VulkanVertexArrayObject.h"

namespace CgEngine {

    struct VulkanQueueFamilyIndices {
        std::optional<uint32_t> graphicsComputeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsComputeFamily.has_value() && presentFamily.has_value();
        }
    };

    struct VulkanSwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class VulkanRenderer : public RendererBackendBase {
    public:
        void init(Window& window) override;
        void shutdown() override;

        void setFramebufferResized() override;

        bool beginFrame(const Window& window) override;
        void endFrame(const Window& window) override;

        void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;
        void beginSwapChainRenderPass() override;
        void endRenderPass() override;

        void beginDynamicRendering(const DynamicRenderingInfo& renderingInfo) override;
        void endDynamicRendering() override;

        void bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) override;
        void bindComputePipeline(const ComputePipeline* computePipeline) override;
        void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;

        void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) override;
        void setPushConstants(const void* data, size_t size) override;

        void injectBarriersForDescriptorSet(const DescriptorSet *descriptorSet) override;

        void transitionImageLayoutFromComputeToShaderReadOnly(Attachment* attachment, ShaderStage stageUsingAttachmentAfterTransition) override;
        void memoryBarrierForVertexBufferAfterCompute(const VertexBuffer* vertexBuffer) override;
        void memoryBarrierForAttachmentAfterComputeToCompute(Attachment* attachment) override;

        void renderUnitQuad() override;
        void renderUnitCube() override;
        void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex, uint32_t instanceCount) override;
        void executeDrawCommand(const VertexArrayObject* vao, uint32_t indexCount, uint32_t baseIndex, uint32_t baseVertex) override;
        void drawArrays(const VertexArrayObject* vao, uint32_t vertexCount) override;

        Texture2D* getWhiteTexture() override;
        Texture2D* getBrdfLUTTexture() override;
        TextureCube* getBlackCubeTexture() override;
        std::pair<TextureCube*, TextureCube*> createEnvironmentMap(const std::string& hdriPath) override;

        const std::vector<VertexBufferLayout> getUnitQuadVertexInputLayout() override;
        const std::vector<VertexBufferLayout> getUnitCubeVertexInputLayout() override;
        PipelineAttachmentInfo getSwapChainAttachmentInfo() override;

        void beginImGuiFrame() override;
        void renderImGuiFrame() override;

        uint64_t getFrameIndex() override;

        void executeImmediateCommand(const std::function<void (const vk::CommandBuffer&)>& lambda) const;

        GraphicsAPI getGraphicsAPI() const override { return GraphicsAPI::Vulkan; }

        const uint32_t getMaxFramesInFlight() const;
        const uint32_t getCurrentFrameIndex() const;
        vk::PhysicalDevice getVkPhysicalDevice() const;
        vk::Device getVkDevice() const;
        vk::CommandBuffer getCurrentCommandBuffer() const;
        VmaAllocator getVmaAllocator() const;
        VulkanDescriptorAllocator& getDescriptorAllocator();
        VulkanSamplerManager& getSamplerManager();

        void deferDestruction(std::function<void()> destroyFn);

    private:
        bool ENABLE_VALIDATION_LAYERS = false;
        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        const std::array<const char*, 1> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        uint32_t currentFrameIndex = 0;
        uint32_t currentSwapChainImageIndex = 0;
        bool framebufferResized = false;
        uint64_t frameIndex = 0;

        vk::PipelineLayout currentPipelineLayout = VK_NULL_HANDLE;
        vk::PipelineBindPoint currentPipelineBindPoint{};
        vk::ShaderStageFlags currentShaderStageFlags{};

        vk::Instance vkInstance;
        vk::detail::DispatchLoaderDynamic vkDispatchLoaderDynamic;
        vk::DebugUtilsMessengerEXT debugMessenger;
        vk::SurfaceKHR vkSurface;
        vk::PhysicalDevice vkPhysicalDevice;
        vk::Device vkDevice;
        vk::Queue vkGraphicsComputeQueue;
        vk::Queue vkPresentQueue;
        struct QueueIndices {
            uint32_t graphicsComputeFamily;
            uint32_t presentFamily;
        } vkQueueIndices;

        vk::SwapchainKHR vkSwapChain;
        std::vector<vk::Image> vkSwapChainImages;
        std::vector<vk::ImageView> vkSwapChainImageViews;
        vk::Format vkSwapChainImageFormat;
        vk::Extent2D vkSwapChainExtent;

        vk::CommandPool vkGraphicsComputeCommandPool;
        vk::CommandPool vkTransientCommandPool;
        std::vector<vk::CommandBuffer> vkGraphicsComputeCommandBuffers;

        std::vector<vk::Semaphore> vkImageAvailableSemaphores;
        std::vector<vk::Semaphore> vkRenderFinishedSemaphores;
        std::vector<vk::Fence> vkInFlightFences;
        std::vector<vk::Fence> vkImagesInFlight;

        VulkanDescriptorAllocator descriptorAllocator{};
        VulkanSamplerManager samplerManager{};
        VulkanBarrierManager barrierManager{};

        struct DeferredDestruction {
            uint64_t safeAtFrameIndex;
            std::function<void()> destroy;
        };
        std::vector<DeferredDestruction> pendingDestructions;
        void flushPendingDestructions();

        VmaAllocator vmaAllocator;

        VulkanVertexArrayObject quadVAO;
        VulkanVertexArrayObject unitCubeVAO;

        VulkanTexture2D whiteTexture{};
        VulkanTexture2D brdfLUT{};
        VulkanTextureCube blackCubeTexture{};

        VulkanDescriptorSetLayout environmentMapComputeDescriptorSetLayout{};
        VulkanComputePipeline computeSphereToCube{};
        VulkanComputePipeline computePrefilterMap{};
        VulkanComputePipeline computeIrradianceMap{};

        vk::DescriptorPool imguiDescriptorPool{};

        bool checkValidationLayerSupport();
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void pickPhysicalDevice();
        void printDeviceInfo();
        uint32_t rateDeviceSuitability(vk::PhysicalDevice dev, vk::SurfaceKHR surf);
        VulkanQueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
        VulkanSwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice dev, vk::SurfaceKHR surf);
        void createLogicalDevice();
        void createSwapChain(const Window& window, vk::SwapchainKHR oldSwapChain);
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes, const Window& window);
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window);
        void createSwapChainImageViews();
        void recreateSwapChain(const Window& window);
        void createCommandPools();
        void createGraphicsComputeCommandBuffers();
        void createSyncObjects();
        void createRenderFinishedSemaphores();
        void createVmaAllocator();
        void initImGui(Window& window);
        void shutdownImGui();
        void beginSwapChainRenderPassInternal(bool clear);

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType,const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData);
    };

}
