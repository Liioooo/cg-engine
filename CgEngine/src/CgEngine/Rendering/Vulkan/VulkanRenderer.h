#pragma once

#include "Rendering/RendererBackendBase.h"
#include <vulkan/vulkan.hpp>

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

        void beginFrame(const Window& window) override;
        void endFrame(const Window& window) override;

        void beginRenderPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;
        void beginSwapChainRenderPass() override;
        void endRenderPass() override;

        void bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) override;
        void bindComputePipeline(const ComputePipeline* computePipeline) override;
        void dispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void clearPass(const RenderPass* renderPass, const Framebuffer* framebuffer) override;

        void bindDescriptorSet(const DescriptorSet* descriptorSet, uint32_t setIndex) override;
        void setPushConstants(const std::array<PushConstants*, 2>& pushConstants, uint32_t pushConstantsCount) override;

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
        const RenderPass* getSwapChainRenderPass() override;

        void beginImGuiFrame() override;
        void renderImGuiFrame() override;

        GraphicsAPI getGraphicsAPI() const override { return GraphicsAPI::Vulkan; }

        vk::PhysicalDevice getVkPhysicalDevice() const;
        vk::Device getVkDevice() const;

    private:
        static inline bool ENABLE_VALIDATION_LAYERS = false;

        static inline const std::array<const char*, 1> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        static inline const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        static inline vk::Instance vkInstance;
        static inline vk::detail::DispatchLoaderDynamic vkDispatchLoaderDynamic;
        static inline vk::DebugUtilsMessengerEXT debugMessenger;
        static inline vk::SurfaceKHR vkSurface;
        static inline vk::PhysicalDevice vkPhysicalDevice;
        static inline vk::Device vkDevice;
        static inline vk::Queue vkGraphicsComputeQueue;
        static inline vk::Queue vkPresentQueue;
        static inline struct QueueIndices {
            uint32_t graphicsComputeFamily;
            uint32_t presentFamily;
        } vkQueueIndices;

        static inline vk::SwapchainKHR vkSwapChain;
        static inline std::vector<vk::Image> vkSwapChainImages;
        static inline std::vector<vk::ImageView> vkSwapChainImageViews;
        static inline vk::Format vkSwapChainImageFormat;
        static inline vk::Extent2D vkSwapChainExtent;

        bool checkValidationLayerSupport();
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void pickPhysicalDevice();
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

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType,const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData);
    };

}
