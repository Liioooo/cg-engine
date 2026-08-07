#include "VulkanHelpers.h"
#include "Asserts.h"
#include "VulkanRenderer.h"
#include "Rendering/Helpers.h"
#include "Rendering/Renderer.h"

namespace CgEngine {

    namespace VulkanHelpers {
        vk::ImageView createImageView2D(vk::Image image, vk::Format format, uint32_t levelCount, uint32_t layerCount, vk::ImageAspectFlags aspectFlags, uint32_t baseArrayLayer) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = image;
            createInfo.viewType = layerCount > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
            createInfo.format = format;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask = aspectFlags;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = levelCount;
            createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
            createInfo.subresourceRange.layerCount = layerCount;

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            auto result = device.createImageView(createInfo);
            CG_ASSERT(result.has_value(), "VulkanHelpers::createImageView2D: Failed to create image view.")
            return result.value;
        }

        vk::ImageView createImageViewCube(vk::Image image, vk::Format format, uint32_t levelCount, vk::ImageAspectFlags aspectFlags) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = image;
            createInfo.viewType = vk::ImageViewType::eCube;
            createInfo.format = format;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask = aspectFlags;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = levelCount;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 6;

            auto device = Renderer::getVulkanBackend()->getVkDevice();

            auto result = device.createImageView(createInfo);
            CG_ASSERT(result.has_value(), "VulkanHelpers::createImageViewCube: Failed to create image view.")
            return result.value;
        }

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
            auto backend = Renderer::getVulkanBackend();

            backend->executeImmediateCommand([&](const vk::CommandBuffer commandBuffer) {
                vk::BufferCopy2 copyRegion{};
                copyRegion.setSize(size);
                copyRegion.setSrcOffset(0);
                copyRegion.setDstOffset(0);

                vk::CopyBufferInfo2 copyRegionInfo{};
                copyRegionInfo.setSrcBuffer(srcBuffer);
                copyRegionInfo.setDstBuffer(dstBuffer);
                copyRegionInfo.setPRegions(&copyRegion);
                copyRegionInfo.setRegionCount(1);

                commandBuffer.copyBuffer2(&copyRegionInfo);
            });
        }

        size_t alignUp(size_t value, size_t alignment) {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        vk::Format findSupportedDepthFormat(const std::vector<vk::Format>& candidates) {
            auto physicalDevice = Renderer::getVulkanBackend()->getVkPhysicalDevice();

            for (vk::Format format : candidates) {
                vk::FormatProperties props = physicalDevice.getFormatProperties(format);

                if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                    return format;
                }
            }
            CG_LOGGING_ERROR("failed to find supported depth format!")
            return vk::Format::eUndefined;
        }

        bool hasFormatStencilComponent(vk::Format format) {
            return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint || format == vk::Format::eD16UnormS8Uint;
        }

        vk::Format attachmentTypeToVulkanColorFormat(AttachmentType attachmentType) {
            switch (attachmentType) {
                case AttachmentType::RGBA8:
                    return vk::Format::eR8G8B8A8Unorm;
                case AttachmentType::RGBA8_SRGB:
                    return vk::Format::eR8G8B8A8Srgb;
                case AttachmentType::BGRA8_SRGB:
                    return vk::Format::eB8G8R8A8Srgb;
                case AttachmentType::RGBA16F:
                    return vk::Format::eR16G16B16A16Sfloat;
                case AttachmentType::RGBA32F:
                    return vk::Format::eR32G32B32A32Sfloat;
                case AttachmentType::RG8:
                    return vk::Format::eR8G8Unorm;
                case AttachmentType::RG16F:
                    return vk::Format::eR16G16Sfloat;
                case AttachmentType::RG32F:
                    return vk::Format::eR32G32Sfloat;
                case AttachmentType::R16F:
                    return vk::Format::eR16Sfloat;
            }

            CG_LOGGING_ERROR("Given attachment type is not a color attachment!")
            return vk::Format::eR8G8B8A8Unorm;;
        }

        AttachmentType vkColorFormatToAttachmentType(vk::Format format) {
            switch (format) {
                case vk::Format::eR8G8B8A8Unorm:
                    return AttachmentType::RGBA8;
                case vk::Format::eR8G8B8A8Srgb:
                    return AttachmentType::RGBA8_SRGB;
                case vk::Format::eB8G8R8A8Srgb:
                    return AttachmentType::BGRA8_SRGB;
                case vk::Format::eR16G16B16A16Sfloat:
                    return AttachmentType::RGBA16F;
                case vk::Format::eR32G32B32A32Sfloat:
                    return AttachmentType::RGBA32F;
                case vk::Format::eR8G8Unorm:
                    return AttachmentType::RG8;
                case vk::Format::eR16G16Sfloat:
                    return AttachmentType::RG16F;
                case vk::Format::eR32G32Sfloat:
                    return AttachmentType::RG32F;
                case vk::Format::eR16Sfloat:
                    return AttachmentType::R16F;
            }

            CG_LOGGING_ERROR("Given Vulkan format is not a color attachment format!")
            return AttachmentType::RGBA8;
        }

        vk::ImageAspectFlags attachmentTypeToAspectFlags(AttachmentType attachmentType) {
            switch (attachmentType) {
                case AttachmentType::Depth:
                    return vk::ImageAspectFlagBits::eDepth;
                case AttachmentType::DepthStencil:
                    return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
                default:
                    return vk::ImageAspectFlagBits::eColor;
            }
        }

        vk::ImageUsageFlags attachmentTypeToUsageFlags(bool usableAsTexture, bool usableAsStorageImage, AttachmentType attachmentType) {
            vk::ImageUsageFlags flags{};

            if (usableAsTexture) {
                flags |= vk::ImageUsageFlagBits::eSampled;
            }

            if (usableAsStorageImage) {
                flags |= vk::ImageUsageFlagBits::eStorage;
            }

            if (attachmentType == AttachmentType::Depth || attachmentType == AttachmentType::DepthStencil) {
                flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
            } else {
                flags |= vk::ImageUsageFlagBits::eColorAttachment;
            }

            return flags;
        }

        VulkanGraphicsShaderInfo loadVulkanGraphicsShader(const std::string &name, ShaderEnv env) {
            CG_LOGGING_DEBUG("Loading Shader: {0}", name)

            const std::vector<uint8_t> vertexSource = Helpers::loadShaderBinaryWithType(name, "vertex", env);
            const std::vector<uint8_t> fragmentSource = Helpers::loadShaderBinaryWithType(name, "fragment", env);
            const std::vector<uint8_t> geometrySource = Helpers::loadShaderBinaryWithType(name, "geometry", env);
            const std::vector<uint8_t> tcsSource = Helpers::loadShaderBinaryWithType(name, "tcs", env);
            const std::vector<uint8_t> tesSource = Helpers::loadShaderBinaryWithType(name, "tes", env);

            return createVulkanGraphicsShaderInfoFromSources(vertexSource, fragmentSource, geometrySource, tcsSource, tesSource);
        }

        VulkanGraphicsShaderInfo loadVulkanGraphicsShader(const std::string &vertex, const std::string &fragment, const std::string &geometry, const std::string &tcs, const std::string &tes, ShaderEnv env) {
            CG_LOGGING_DEBUG("Loading Shader: {0}", vertex)

            const std::vector<uint8_t> vertexSource = Helpers::loadShaderBinary(vertex + ".spv", env);
            const std::vector<uint8_t> fragmentSource = Helpers::loadShaderBinary(fragment + ".spv", env);
            const std::vector<uint8_t> geometrySource = Helpers::loadShaderBinary(geometry + ".spv", env);
            const std::vector<uint8_t> tcsSource = Helpers::loadShaderBinary(tcs + ".spv", env);
            const std::vector<uint8_t> tesSource = Helpers::loadShaderBinary(tes + ".spv", env);

            return createVulkanGraphicsShaderInfoFromSources(vertexSource, fragmentSource, geometrySource, tcsSource, tesSource);
        }

        VulkanGraphicsShaderInfo createVulkanGraphicsShaderInfoFromSources(const std::vector<uint8_t>& vertexSource, const std::vector<uint8_t>& fragmentSource, const std::vector<uint8_t>& geometrySource, const std::vector<uint8_t>& tcsSource, const std::vector<uint8_t>& tesSource) {
            VulkanGraphicsShaderInfo info{};

            if (!vertexSource.empty()) {
                info.vertexModule = createVulkanShaderModule(vertexSource);

                vk::PipelineShaderStageCreateInfo stageInfo{};
                stageInfo.stage = vk::ShaderStageFlagBits::eVertex;
                stageInfo.module = info.vertexModule;
                stageInfo.pName = "main";

                info.shaderStages.push_back(stageInfo);
            }

            if (!fragmentSource.empty()) {
                info.fragmentModule = createVulkanShaderModule(fragmentSource);

                vk::PipelineShaderStageCreateInfo stageInfo{};
                stageInfo.stage = vk::ShaderStageFlagBits::eFragment;
                stageInfo.module = info.fragmentModule;
                stageInfo.pName = "main";

                info.shaderStages.push_back(stageInfo);
            }

            if (!geometrySource.empty()) {
                info.geometryModule = createVulkanShaderModule(geometrySource);

                vk::PipelineShaderStageCreateInfo stageInfo{};
                stageInfo.stage = vk::ShaderStageFlagBits::eGeometry;
                stageInfo.module = info.geometryModule;
                stageInfo.pName = "main";

                info.shaderStages.push_back(stageInfo);
            }

            if (!tesSource.empty()) {
                info.tesModule = createVulkanShaderModule(tesSource);

                vk::PipelineShaderStageCreateInfo stageInfo{};
                stageInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
                stageInfo.module = info.tesModule;
                stageInfo.pName = "main";

                info.shaderStages.push_back(stageInfo);
            }

            if (!tcsSource.empty()) {
                info.tcsModule = createVulkanShaderModule(tcsSource);

                vk::PipelineShaderStageCreateInfo stageInfo{};
                stageInfo.stage = vk::ShaderStageFlagBits::eTessellationControl;
                stageInfo.module = info.tcsModule;
                stageInfo.pName = "main";

                info.shaderStages.push_back(stageInfo);
            }

            return info;
        }

        vk::ShaderModule createVulkanShaderModule(const std::vector<uint8_t>& binary) {
            auto device = Renderer::getVulkanBackend()->getVkDevice();

            vk::ShaderModuleCreateInfo info{};
            info.setCodeSize(binary.size());
            info.setPCode(reinterpret_cast<const uint32_t*>(binary.data()));

            auto result = device.createShaderModule(info);
            CG_ASSERT(result.has_value(), "VulkanHelpers::createVulkanShaderModule: Failed to create shader module.")
            return result.value;
        }

        void destroyVulkanGraphicsShaderModules(VulkanGraphicsShaderInfo& shaderInfo) {
            auto device = Renderer::getVulkanBackend()->getVkDevice();

            device.destroyShaderModule(shaderInfo.vertexModule);
            device.destroyShaderModule(shaderInfo.fragmentModule);
            device.destroyShaderModule(shaderInfo.geometryModule);
            device.destroyShaderModule(shaderInfo.tesModule);
            device.destroyShaderModule(shaderInfo.tcsModule);

            shaderInfo.vertexModule = VK_NULL_HANDLE;
            shaderInfo.fragmentModule = VK_NULL_HANDLE;
            shaderInfo.geometryModule = VK_NULL_HANDLE;
            shaderInfo.tesModule = VK_NULL_HANDLE;
            shaderInfo.tcsModule = VK_NULL_HANDLE;
        }

        VulkanPipelineVertexInputInfo getVulkanPipelineVertexInputInfo(const std::vector<VertexBufferLayout>& vertexInputLayout) {
            VulkanPipelineVertexInputInfo info{};

            info.bindings.resize(vertexInputLayout.size());

            for (int i = 0; i < vertexInputLayout.size(); ++i) {
                const auto& layoutItem = vertexInputLayout[i];
                info.bindings[i].setBinding(i);
                info.bindings[i].setStride(layoutItem.stride);
                info.bindings[i].setInputRate(vk::VertexInputRate::eVertex);
            }

            info.vertexInputInfo.setVertexBindingDescriptions(info.bindings);

            uint32_t shaderLocation = 0;
            for (int binding = 0; binding < vertexInputLayout.size(); ++binding) {
                for (const auto& element: vertexInputLayout[binding].bufferElements) {
                    vk::VertexInputAttributeDescription description{};
                    description.setBinding(binding);
                    description.setLocation(shaderLocation);
                    description.setOffset(element.offset);
                    description.setFormat(shaderDataTypeToVulkanFormat(element.dataType));

                    info.attributes.push_back(description);
                    shaderLocation++;
                }
            }

            info.vertexInputInfo.setVertexAttributeDescriptions(info.attributes);

            return info;
        }

        vk::Format shaderDataTypeToVulkanFormat(const ShaderDataType type) {
            switch (type) {
                case ShaderDataType::Float:  return vk::Format::eR32Sfloat;
                case ShaderDataType::Float2: return vk::Format::eR32G32Sfloat;
                case ShaderDataType::Float3: return vk::Format::eR32G32B32Sfloat;
                case ShaderDataType::Float4: return vk::Format::eR32G32B32A32Sfloat;

                case ShaderDataType::Int:    return vk::Format::eR32Sint;
                case ShaderDataType::Int2:   return vk::Format::eR32G32Sint;
                case ShaderDataType::Int3:   return vk::Format::eR32G32B32Sint;
                case ShaderDataType::Int4:   return vk::Format::eR32G32B32A32Sint;
            }

            CG_LOGGING_ERROR("Given ShaderDataType not supported!")
            return vk::Format::eR32Sfloat;;
        }

        vk::PrimitiveTopology drawModeToVulkanPrimitiveTopology(DrawMode mode) {
            switch (mode) {
                case DrawMode::Lines:  return vk::PrimitiveTopology::eLineList;
                case DrawMode::Triangles:  return vk::PrimitiveTopology::eTriangleList;
                case DrawMode::Patches: return vk::PrimitiveTopology::ePatchList;
            }
        }

        vk::CompareOp depthCompareOperatorToVulkan(DepthCompareOperator op) {
            switch (op) {
                case DepthCompareOperator::Never:
                    return vk::CompareOp::eNever;
                case DepthCompareOperator::Less:
                    return vk::CompareOp::eLess;
                case DepthCompareOperator::Equal:
                    return vk::CompareOp::eEqual;
                case DepthCompareOperator::LessOrEqual:
                    return vk::CompareOp::eLessOrEqual;
                case DepthCompareOperator::Greater:
                    return vk::CompareOp::eGreater;
                case DepthCompareOperator::NotEqual:
                    return vk::CompareOp::eNotEqual;
                case DepthCompareOperator::GreaterOrEqual:
                    return vk::CompareOp::eGreaterOrEqual;
                case DepthCompareOperator::Always:
                    return vk::CompareOp::eAlways;
                default:
                    return vk::CompareOp::eLess;
            }
        }

        vk::BlendOp blendingEquationToVulkan(BlendingEquation eq) {
            switch (eq){
                case BlendingEquation::Add:
                    return vk::BlendOp::eAdd;
                case BlendingEquation::Subtract:
                    return vk::BlendOp::eSubtract;
                case BlendingEquation::ReverseSubtract:
                    return vk::BlendOp::eReverseSubtract;
                case BlendingEquation::Min:
                    return vk::BlendOp::eMin;
                case BlendingEquation::Max:
                    return vk::BlendOp::eMax;
            }
            return vk::BlendOp::eAdd;
        }

        vk::BlendFactor blendingFunctionToVulkan(BlendingFunction fn) {
            switch (fn) {
                case BlendingFunction::Zero:
                    return vk::BlendFactor::eZero;
                case BlendingFunction::One:
                    return vk::BlendFactor::eOne;
                case BlendingFunction::SrcColor:
                    return vk::BlendFactor::eSrcColor;
                case BlendingFunction::OneMinusSrcColor:
                    return vk::BlendFactor::eOneMinusSrcColor;
                case BlendingFunction::SrcAlpha:
                    return vk::BlendFactor::eSrcAlpha;
                case BlendingFunction::OneMinusSrcAlpha:
                    return vk::BlendFactor::eOneMinusSrcAlpha;
                case BlendingFunction::DestAlpha:
                    return vk::BlendFactor::eDstAlpha;
                case BlendingFunction::OneMinusDestAlpha:
                    return vk::BlendFactor::eOneMinusDstAlpha;
                case BlendingFunction::DestColor:
                    return vk::BlendFactor::eDstColor;
                case BlendingFunction::OneMinusDestColor:
                    return vk::BlendFactor::eOneMinusDstColor;
            }

            return vk::BlendFactor::eZero;
        }

        vk::Format depthStencilAttachmentFormatToVulkanFormat(DepthStencilAttachmentFormat format) {
            switch (format) {
                case DepthStencilAttachmentFormat::Depth32Float:
                    return vk::Format::eD32Sfloat;
                case DepthStencilAttachmentFormat::Depth32FloatStencil8:
                    return vk::Format::eD32SfloatS8Uint;
                case DepthStencilAttachmentFormat::Depth24Stencil8:
                    return vk::Format::eD24UnormS8Uint;
            }

            CG_LOGGING_ERROR("VulkanHelpers::depthStencilAttachmentFormatToVulkanFormat: Unknown DepthStencilAttachmentFormat value.")
            return vk::Format::eD32Sfloat;
        }

        vk::ShaderStageFlags descriptorSetLayoutBindingUsageToVulkanShaderStageFlags(DescriptorSetLayoutBindingUsage usage) {
            vk::ShaderStageFlags flags{};

            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Compute)) {
                flags |= vk::ShaderStageFlagBits::eCompute;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::TCS)) {
                flags |= vk::ShaderStageFlagBits::eTessellationControl;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::TES)) {
                flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Geometry)) {
                flags |= vk::ShaderStageFlagBits::eGeometry;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Fragment)) {
                flags |= vk::ShaderStageFlagBits::eFragment;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Vertex)) {
                flags |= vk::ShaderStageFlagBits::eVertex;
            }

            return flags;
        }

        vk::PipelineStageFlags2 descriptorSetLayoutBindingUsageToVulkanPipelineStageFlags(DescriptorSetLayoutBindingUsage usage) {
            vk::PipelineStageFlags2 flags{};

            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Compute)) {
                flags |= vk::PipelineStageFlagBits2::eComputeShader;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::TCS)) {
                flags |= vk::PipelineStageFlagBits2::eTessellationControlShader;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::TES)) {
                flags |= vk::PipelineStageFlagBits2::eTessellationEvaluationShader;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Geometry)) {
                flags |= vk::PipelineStageFlagBits2::eGeometryShader;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Fragment)) {
                flags |= vk::PipelineStageFlagBits2::eFragmentShader;
            }
            if (hasFlag(usage, DescriptorSetLayoutBindingUsage::Vertex)) {
                flags |= vk::PipelineStageFlagBits2::eVertexShader;
            }

            return flags;
        }

        vk::Format textureFormatToVulkanFormat(TextureFormat format) {
            switch (format) {
                case TextureFormat::R:
                    return vk::Format::eR8Unorm;
                case TextureFormat::RG:
                    return vk::Format::eR8G8Unorm;
                case TextureFormat::RGBA:
                    return vk::Format::eR8G8B8A8Unorm;
                case TextureFormat::RGBA_SRGB:
                    return vk::Format::eR8G8B8A8Srgb;
                case TextureFormat::RedFloat16:
                    return vk::Format::eR16Sfloat;
                case TextureFormat::RedFloat32:
                    return vk::Format::eR32Sfloat;
                case TextureFormat::RedGreenFloat16:
                    return vk::Format::eR16G16Sfloat;
                case TextureFormat::RedGreenFloat32:
                    return vk::Format::eR32G32Sfloat;
                case TextureFormat::Float16A:
                    return vk::Format::eR16G16B16A16Sfloat;
                case TextureFormat::Float32A:
                    return vk::Format::eR32G32B32A32Sfloat;
            }

            CG_LOGGING_ERROR("VulkanHelpers::textureFormatToVulkanFormat: Unknown TextureFormat value.")
            return vk::Format::eR8G8B8A8Unorm;
        }

        VulkanComputeShaderInfo loadVulkanComputeShader(const std::string &name, ShaderEnv env) {
            std::vector<uint8_t> source = Helpers::loadShaderBinaryWithType(name, "comp", env);
            return createVulkanComputeShaderInfoFromSource(source);
        }

        VulkanComputeShaderInfo loadVulkanCustomComputeShader(const std::string &name) {
            std::vector<uint8_t> source = Helpers::loadShaderBinary(name + ".spv", ShaderEnv::Custom);
            return createVulkanComputeShaderInfoFromSource(source);
        }

        VulkanComputeShaderInfo createVulkanComputeShaderInfoFromSource(const std::vector<uint8_t>& source) {
            VulkanComputeShaderInfo info{};

            info.computeModule = createVulkanShaderModule(source);

            vk::PipelineShaderStageCreateInfo stageInfo{};
            stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
            stageInfo.module = info.computeModule;
            stageInfo.pName = "main";

            info.shaderStage = stageInfo;

            return info;
        }

        void destroyVulkanComputeShaderModule(VulkanComputeShaderInfo &shaderInfo) {
            auto device = Renderer::getVulkanBackend()->getVkDevice();
            device.destroyShaderModule(shaderInfo.computeModule);
            shaderInfo.computeModule = VK_NULL_HANDLE;
        }
    }
}
