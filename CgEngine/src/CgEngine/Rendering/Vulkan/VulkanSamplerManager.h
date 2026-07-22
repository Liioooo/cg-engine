#pragma once

#include <vulkan/vulkan.hpp>
#include "CgEngineSharedUtils/Enums.h"

namespace CgEngine {
    class VulkanSamplerManager {
    public:
        VulkanSamplerManager() = default;

        void init(vk::Device device);
        void shutdown();

        vk::Sampler getSampler(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor);

    private:
        vk::Device vkDevice;
        std::unordered_map<uint32_t, vk::Sampler> samplers;

        static uint32_t getSamplerKey(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor);
        vk::Sampler createSampler(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor);
    };
}