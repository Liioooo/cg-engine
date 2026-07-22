#include "VulkanSamplerManager.h"

#include "Application.h"
#include "Asserts.h"
#include "Logging.h"
#include "VulkanRenderer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {

    void VulkanSamplerManager::init(vk::Device device) {
        vkDevice = device;
        samplers.clear();
    }

    void VulkanSamplerManager::shutdown() {
        for (auto& [key, sampler] : samplers) {
            vkDevice.destroySampler(sampler);
        }
        samplers.clear();
    }

    vk::Sampler VulkanSamplerManager::getSampler(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor) {
        uint32_t key = getSamplerKey(textureWrap, mipMapFiltering, textureBorderColor);

        auto it = samplers.find(key);
        if (it != samplers.end()) {
            return it->second;
        }

        vk::Sampler sampler = createSampler(textureWrap, mipMapFiltering, textureBorderColor);
        samplers[key] = sampler;
        return sampler;
    }

    uint32_t VulkanSamplerManager::getSamplerKey(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor) {
        return static_cast<uint32_t>(textureWrap) | (static_cast<uint32_t>(mipMapFiltering) << 8) | (static_cast<uint32_t>(textureBorderColor) << 16);
    }

    vk::Sampler VulkanSamplerManager::createSampler(TextureWrap textureWrap, MipMapFiltering mipMapFiltering, TextureBorderColor textureBorderColor) {
        vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
        switch (textureWrap) {
            case TextureWrap::Repeat:
                addressMode = vk::SamplerAddressMode::eRepeat;
                break;
            case TextureWrap::Clamp:
                addressMode = vk::SamplerAddressMode::eClampToEdge;
                break;
            case TextureWrap::ClampBorder:
                addressMode = vk::SamplerAddressMode::eClampToBorder;
                break;
        }

        vk::BorderColor borderColor = vk::BorderColor::eFloatOpaqueBlack;
        switch (textureBorderColor) {
            case TextureBorderColor::OpaqueBlack:
                borderColor = vk::BorderColor::eFloatOpaqueBlack;
                break;
            case TextureBorderColor::OpaqueWhite:
                borderColor = vk::BorderColor::eFloatOpaqueWhite;
                break;
        }

        vk::Filter filter = vk::Filter::eNearest;
        vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eNearest;
        float maxLod = 0.0f;
        vk::Bool32 anisotropyEnable = VK_FALSE;
        float maxAnisotropy = 1.0f;

        switch (mipMapFiltering) {
            case MipMapFiltering::Nearest:
                filter = vk::Filter::eNearest;
                mipmapMode = vk::SamplerMipmapMode::eNearest;
                maxLod = 0.0f;
                break;
            case MipMapFiltering::Bilinear:
                filter = vk::Filter::eLinear;
                mipmapMode = vk::SamplerMipmapMode::eNearest;
                maxLod = 0.0f;
                break;
            case MipMapFiltering::Trilinear:
                filter = vk::Filter::eLinear;
                mipmapMode = vk::SamplerMipmapMode::eLinear;
                maxLod = VK_LOD_CLAMP_NONE;
                break;
            case MipMapFiltering::Anisotropic: {
                filter = vk::Filter::eLinear;
                mipmapMode = vk::SamplerMipmapMode::eLinear;
                maxLod = VK_LOD_CLAMP_NONE;
                anisotropyEnable = VK_TRUE;

                vk::PhysicalDeviceProperties props = Renderer::getVulkanBackend()->getVkPhysicalDevice().getProperties();
                maxAnisotropy = std::min(Application::get().getApplicationOptions().anisotropicFiltering, props.limits.maxSamplerAnisotropy);
                break;
            }
        }

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = filter;
        samplerInfo.minFilter = filter;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = anisotropyEnable;
        samplerInfo.maxAnisotropy = maxAnisotropy;
        samplerInfo.borderColor = borderColor;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = maxLod;

        auto result = vkDevice.createSampler(samplerInfo);
        if (!result.has_value()) {
            CG_LOGGING_ERROR("VulkanSamplerManager: Failed to create sampler!")
        }

        return result.value;
    }

}