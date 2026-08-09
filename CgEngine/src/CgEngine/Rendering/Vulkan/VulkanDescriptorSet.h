#pragma once
#include "Rendering/DescriptorSet.h"
#include <vulkan/vulkan.hpp>
#include "VulkanDescriptorSetLayout.h"

namespace CgEngine {
    class VulkanDescriptorSet : public DescriptorSet {
    public:
        VulkanDescriptorSet() = default;
        explicit VulkanDescriptorSet(const DescriptorSetSpecification& spec);
        explicit VulkanDescriptorSet(const DescriptorSetLayout* layout);

        ~VulkanDescriptorSet() override = default;

        VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept;
        VulkanDescriptorSet& operator=(VulkanDescriptorSet&& other) noexcept;

        VulkanDescriptorSet(VulkanDescriptorSet& other) = delete;
        VulkanDescriptorSet& operator=(VulkanDescriptorSet& other) = delete;

        bool isReady() const override;
        void recreate() override;
        void reconfigure(const DescriptorSetSpecification& spec) override;

        vk::DescriptorSet getVulkanDescriptorSet() const;
        std::vector<uint32_t> getDynamicBufferOffsets() const;
        const VulkanDescriptorSetLayout* getLayout() const;
        const std::vector<DescSetAttachmentTextureBinding>& getAttachmentTextureBindings() const;
        const std::vector<DescSetAttachmentImageBinding>& getAttachmentImageBindings() const;

    private:
        DescriptorSetSpecification specification{};
        std::vector<vk::DescriptorSet> descriptorSets;
        size_t lastUpdatedSlot = 0;
        uint64_t lastWrittenOnFrameIndex = 0;
        std::map<uint32_t, std::function<uint32_t()>> dynamicBufferOffsetGetters{};

        void growToMaxFramesInFlight();
        void rebuildDynamicBufferOffsetGetters();
        void applyWrites(vk::DescriptorSet target);
    };
}
