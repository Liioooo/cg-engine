#include "VulkanDescriptorSet.h"
#include "Asserts.h"
#include "VulkanAttachment.h"
#include "VulkanImmutableShaderStorageBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanShaderStorageBuffer.h"
#include "VulkanTextureCube.h"
#include "VulkanUniformBuffer.h"
#include "VulkanVertexBuffer.h"
#include "Rendering/Renderer.h"

namespace CgEngine {
    VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetSpecification &spec) : specification(spec) {
        CG_ASSERT(spec.layout != nullptr, "DescriptorSet: Layout is null!")

        descriptorSets.push_back(Renderer::getVulkanBackend()->getDescriptorAllocator().allocateDescriptorSet(static_cast<const VulkanDescriptorSetLayout*>(spec.layout)->getDescriptorSetLayout()));
        rebuildDynamicBufferOffsetGetters();
        applyWrites(descriptorSets[lastUpdatedSlot]);
    }

    VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetLayout *layout) {
        CG_ASSERT(layout != nullptr, "DescriptorSet: Layout is null!")
        specification.layout = layout;
    }

    VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet &&other) noexcept : DescriptorSet(std::move(other)) {
        specification = other.specification;
        descriptorSets = std::move(other.descriptorSets);
        lastUpdatedSlot = other.lastUpdatedSlot;
        lastWrittenOnFrameIndex = other.lastWrittenOnFrameIndex;
        dynamicBufferOffsetGetters = std::move(other.dynamicBufferOffsetGetters);
    }

    VulkanDescriptorSet & VulkanDescriptorSet::operator=(VulkanDescriptorSet &&other) noexcept {
        if (this != &other) {
            DescriptorSet::operator=(std::move(other));

            specification = other.specification;
            descriptorSets = std::move(other.descriptorSets);
            lastUpdatedSlot = other.lastUpdatedSlot;
            lastWrittenOnFrameIndex = other.lastWrittenOnFrameIndex;
            dynamicBufferOffsetGetters = std::move(other.dynamicBufferOffsetGetters);
        }
        return *this;
    }

    bool VulkanDescriptorSet::isReady() const {
        return !descriptorSets.empty();
    }

    void VulkanDescriptorSet::recreate() {
        CG_ASSERT(isReady(), "DescriptorSet: Cannot recreate a descriptor set that is not ready!")

        rebuildDynamicBufferOffsetGetters();
        growToMaxFramesInFlight();

        if (lastWrittenOnFrameIndex != Renderer::getFrameIndex()) {
            lastUpdatedSlot = (lastUpdatedSlot + 1) % Renderer::getVulkanBackend()->getMaxFramesInFlight();
            lastWrittenOnFrameIndex = Renderer::getFrameIndex();
        }

        applyWrites(descriptorSets[lastUpdatedSlot]);
    }

    void VulkanDescriptorSet::reconfigure(const DescriptorSetSpecification &spec) {
        CG_ASSERT(spec.layout == nullptr || spec.layout == specification.layout, "DescriptorSet: Cannot change layout during reconfiguration!")

        auto savedLayout = specification.layout;
        specification = spec;
        specification.layout = savedLayout;

        if (descriptorSets.empty()) {
            descriptorSets.push_back(Renderer::getVulkanBackend()->getDescriptorAllocator().allocateDescriptorSet(static_cast<const VulkanDescriptorSetLayout*>(specification.layout)->getDescriptorSetLayout()));
            rebuildDynamicBufferOffsetGetters();
            applyWrites(descriptorSets[lastUpdatedSlot]);
            return;
        }

        recreate();
    }

    void VulkanDescriptorSet::growToMaxFramesInFlight() {
        uint32_t maxFramesInFlight = Renderer::getVulkanBackend()->getMaxFramesInFlight();
        if (descriptorSets.size() >= maxFramesInFlight) {
            return;
        }

        vk::DescriptorSetLayout vkLayout = static_cast<const VulkanDescriptorSetLayout*>(specification.layout)->getDescriptorSetLayout();
        auto& allocator = Renderer::getVulkanBackend()->getDescriptorAllocator();

        descriptorSets.clear();
        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            descriptorSets.push_back(allocator.allocateDescriptorSet(vkLayout));
        }
    }

    void VulkanDescriptorSet::rebuildDynamicBufferOffsetGetters() {
        dynamicBufferOffsetGetters.clear();

        for (const auto& binding: specification.uboBindings) {
            const auto* ub = static_cast<const VulkanUniformBuffer*>(binding.ubo);
            dynamicBufferOffsetGetters[binding.bindingPoint] = [ub] {
                return static_cast<uint32_t>(ub->getOffsetForCurrentFrame());
            };
        }

        for (const auto& binding: specification.ssboBindings) {
            const auto* ssbo = static_cast<const VulkanShaderStorageBuffer*>(binding.ssbo);
            dynamicBufferOffsetGetters[binding.bindingPoint] = [ssbo] {
                return static_cast<uint32_t>(ssbo->getOffsetForCurrentFrame());
            };
        }

        for (const auto& binding: specification.vertexBufferSSBOBindings) {
            const auto* vb = static_cast<const VulkanVertexBuffer*>(binding.vertexBuffer);
            dynamicBufferOffsetGetters[binding.bindingPoint] = [vb] {
                return static_cast<uint32_t>(vb->getOffsetForCurrentFrame());
            };
        }
    }

    void VulkanDescriptorSet::applyWrites(vk::DescriptorSet target) {
        std::vector<vk::WriteDescriptorSet> writes;
        writes.reserve(
            specification.uboBindings.size() +
            specification.ssboBindings.size() +
            specification.immutableSsboBindings.size() +
            specification.texture2DBindings.size() +
            specification.textureCubeBindings.size() +
            specification.attachmentTextureBindings.size() +
            specification.attachmentImageBindings.size() +
            specification.vertexBufferSSBOBindings.size()
        );

        size_t bufferInfoCount =
            specification.uboBindings.size() +
            specification.ssboBindings.size() +
            specification.immutableSsboBindings.size() +
            specification.vertexBufferSSBOBindings.size();

        size_t textureInfoCount = specification.attachmentTextureBindings.size() + specification.attachmentImageBindings.size();
        for (const auto& binding : specification.texture2DBindings) {
            textureInfoCount += binding.texture != nullptr ? 1 : binding.textureArray.size();
        }
        for (const auto& binding : specification.textureCubeBindings) {
            textureInfoCount += binding.texture != nullptr ? 1 : binding.textureArray.size();
        }

        // bufferInfos/textureInfos must not reallocate: writes below store pointers into these vectors
        std::vector<vk::DescriptorBufferInfo> bufferInfos{};
        bufferInfos.reserve(bufferInfoCount);
        std::vector<vk::DescriptorImageInfo> textureInfos{};
        textureInfos.reserve(textureInfoCount);

        for (const auto& binding: specification.uboBindings) {
            const auto* ub = static_cast<const VulkanUniformBuffer*>(binding.ubo);

            auto& bufferInfo = bufferInfos.emplace_back();
            bufferInfo.buffer = ub->getVulkanBufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = ub->getAlignedFrameSize();

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic);
            write.setDescriptorCount(1);
            write.setPBufferInfo(&bufferInfo);

            writes.push_back(write);
        }

        for (const auto& binding: specification.ssboBindings) {
            const auto* ssbo = static_cast<const VulkanShaderStorageBuffer*>(binding.ssbo);

            auto& bufferInfo = bufferInfos.emplace_back();
            bufferInfo.buffer = ssbo->getVulkanBufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = ssbo->getAlignedFrameSize();

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eStorageBufferDynamic);
            write.setDescriptorCount(1);
            write.setPBufferInfo(&bufferInfo);

            writes.push_back(write);
        }

        for (const auto& binding: specification.immutableSsboBindings) {
            const auto* ssbo = static_cast<const VulkanImmutableShaderStorageBuffer*>(binding.ssbo);

            auto& bufferInfo = bufferInfos.emplace_back();
            bufferInfo.buffer = ssbo->getVulkanBufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eStorageBuffer);
            write.setDescriptorCount(1);
            write.setPBufferInfo(&bufferInfo);

            writes.push_back(write);
        }

        for (const auto& binding: specification.texture2DBindings) {
            if (binding.texture != nullptr) {
                const auto* tex = static_cast<const VulkanTexture2D*>(binding.texture);

                auto& textureInfo = textureInfos.emplace_back();
                textureInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                textureInfo.imageView = tex->getVulkanImageView();
                textureInfo.sampler = tex->getVulkanSampler();

                vk::WriteDescriptorSet write{};
                write.setDstSet(target);
                write.setDstArrayElement(0);
                write.setDstBinding(binding.bindingPoint);
                write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                write.setDescriptorCount(1);
                write.setPImageInfo(&textureInfo);

                writes.push_back(write);
            } else {
                size_t firstTextureInfoIndex = textureInfos.size();

                for (const auto tex : binding.textureArray) {
                    const auto* texVk = static_cast<const VulkanTexture2D*>(tex);

                    auto& textureInfo = textureInfos.emplace_back();
                    textureInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    textureInfo.imageView = texVk->getVulkanImageView();
                    textureInfo.sampler = texVk->getVulkanSampler();
                }

                vk::WriteDescriptorSet write{};
                write.setDstSet(target);
                write.setDstArrayElement(0);
                write.setDstBinding(binding.bindingPoint);
                write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                write.setDescriptorCount(static_cast<uint32_t>(binding.textureArray.size()));
                write.setPImageInfo(&textureInfos[firstTextureInfoIndex]);

                writes.push_back(write);
            }
        }

        for (const auto& binding: specification.textureCubeBindings) {
            if (binding.texture != nullptr) {
                const auto* tex = static_cast<const VulkanTextureCube*>(binding.texture);

                auto& textureInfo = textureInfos.emplace_back();
                textureInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                textureInfo.imageView = tex->getVulkanImageView();
                textureInfo.sampler = tex->getVulkanSampler();

                vk::WriteDescriptorSet write{};
                write.setDstSet(target);
                write.setDstArrayElement(0);
                write.setDstBinding(binding.bindingPoint);
                write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                write.setDescriptorCount(1);
                write.setPImageInfo(&textureInfo);

                writes.push_back(write);
            } else {
                size_t firstTextureInfoIndex = textureInfos.size();

                for (const auto tex : binding.textureArray) {
                    const auto* texVk = static_cast<const VulkanTextureCube*>(tex);

                    auto& textureInfo = textureInfos.emplace_back();
                    textureInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                    textureInfo.imageView = texVk->getVulkanImageView();
                    textureInfo.sampler = texVk->getVulkanSampler();
                }

                vk::WriteDescriptorSet write{};
                write.setDstSet(target);
                write.setDstArrayElement(0);
                write.setDstBinding(binding.bindingPoint);
                write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                write.setDescriptorCount(static_cast<uint32_t>(binding.textureArray.size()));
                write.setPImageInfo(&textureInfos[firstTextureInfoIndex]);

                writes.push_back(write);
            }
        }

        for (const auto& binding: specification.attachmentTextureBindings) {
            if (binding.attachment == nullptr) {
                continue;
            }

            const auto* attachment = static_cast<const VulkanAttachment*>(binding.attachment);

            auto& textureInfo = textureInfos.emplace_back();
            textureInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            textureInfo.imageView = binding.allLayers ? attachment->getVulkanImageView() : attachment->getVulkanLayerImageView(binding.layer);
            textureInfo.sampler = attachment->getVulkanSampler();

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            write.setDescriptorCount(1);
            write.setPImageInfo(&textureInfo);

            writes.push_back(write);
        }

        for (const auto& binding: specification.attachmentImageBindings) {
            if (binding.attachment == nullptr) {
                continue;
            }

            const auto* attachment = static_cast<const VulkanAttachment*>(binding.attachment);

            auto& textureInfo = textureInfos.emplace_back();
            textureInfo.imageLayout = vk::ImageLayout::eGeneral;
            textureInfo.imageView = binding.allLayers ? attachment->getVulkanImageView() : attachment->getVulkanLayerImageView(binding.layer);
            textureInfo.sampler = VK_NULL_HANDLE;

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eStorageImage);
            write.setDescriptorCount(1);
            write.setPImageInfo(&textureInfo);

            writes.push_back(write);
        }

        for (const auto& binding: specification.vertexBufferSSBOBindings) {
            const auto* vb = static_cast<const VulkanVertexBuffer*>(binding.vertexBuffer);

            auto& bufferInfo = bufferInfos.emplace_back();
            bufferInfo.buffer = vb->getVulkanBufferHandle();
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;

            vk::WriteDescriptorSet write{};
            write.setDstSet(target);
            write.setDstArrayElement(0);
            write.setDstBinding(binding.bindingPoint);
            write.setDescriptorType(vk::DescriptorType::eStorageBufferDynamic);
            write.setDescriptorCount(1);
            write.setPBufferInfo(&bufferInfo);

            writes.push_back(write);
        }

        if (!writes.empty()) {
            Renderer::getVulkanBackend()->getVkDevice().updateDescriptorSets(writes, {});
        }
    }

    vk::DescriptorSet VulkanDescriptorSet::getVulkanDescriptorSet() const {
        CG_ASSERT(isReady(), "DescriptorSet: Cannot get Vulkan descriptor set from a descriptor set that is not ready!")
        return descriptorSets[lastUpdatedSlot];
    }

    std::vector<uint32_t> VulkanDescriptorSet::getDynamicBufferOffsets() const {
        CG_ASSERT(isReady(), "DescriptorSet: Cannot get dynamic buffer offsets from a descriptor set that is not ready!")

        std::vector<uint32_t> dynamicBufferOffsets;
        dynamicBufferOffsets.reserve(dynamicBufferOffsetGetters.size());

        for (const auto& getter: dynamicBufferOffsetGetters | std::views::values) {
            dynamicBufferOffsets.push_back(getter());
        }

        return dynamicBufferOffsets;
    }

    const VulkanDescriptorSetLayout* VulkanDescriptorSet::getLayout() const {
        return static_cast<const VulkanDescriptorSetLayout*>(specification.layout);
    }

    const std::vector<DescSetAttachmentTextureBinding>& VulkanDescriptorSet::getAttachmentTextureBindings() const {
        return specification.attachmentTextureBindings;
    }

    const std::vector<DescSetAttachmentImageBinding>& VulkanDescriptorSet::getAttachmentImageBindings() const {
        return specification.attachmentImageBindings;
    }

    const std::vector<DescSetVertexBufferSSBOBinding> & VulkanDescriptorSet::getVertexBufferSsboBindings() const {
        return specification.vertexBufferSSBOBindings;
    }
}
