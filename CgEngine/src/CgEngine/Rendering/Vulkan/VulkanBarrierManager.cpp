#include "VulkanBarrierManager.h"

namespace CgEngine {
    void VulkanBarrierManager::requestAttachmentState(VulkanAttachment *attachment, VulkanAttachmentState newState, bool allLayers, uint32_t layer) {
        attachmentStates.push_back({attachment, newState, allLayers, layer});
    }

    void VulkanBarrierManager::flushBarriers(vk::CommandBuffer commandBuffer) {
        std::vector<vk::ImageMemoryBarrier2> barriers;

        for (auto& attachmentState : attachmentStates) {
            std::vector<VulkanAttachmentState>& subresourceStates = attachmentState.attachment->getSubresourceStates();
            vk::Image image = attachmentState.attachment->getVulkanImage();
            vk::ImageAspectFlags aspectMask = attachmentState.attachment->getVulkanAspectFlags();

            uint32_t firstLayer = attachmentState.allLayers ? 0 : attachmentState.layer;
            uint32_t touchedLayerCount = attachmentState.allLayers ? attachmentState.attachment->getLayerCount() : 1;

            uint32_t groupStart = firstLayer;
            uint32_t groupLength = 0;
            VulkanAttachmentState groupOldState{};

            auto flushGroup = [&]() {
                if (groupLength == 0) {
                    return;
                }

                vk::ImageMemoryBarrier2& barrier = barriers.emplace_back();
                barrier.setSrcStageMask(groupOldState.stage);
                barrier.setSrcAccessMask(groupOldState.access);
                barrier.setDstStageMask(attachmentState.newState.stage);
                barrier.setDstAccessMask(attachmentState.newState.access);
                barrier.setOldLayout(groupOldState.imageLayout);
                barrier.setNewLayout(attachmentState.newState.imageLayout);
                barrier.setImage(image);
                barrier.subresourceRange.setAspectMask(aspectMask);
                barrier.subresourceRange.setBaseMipLevel(0);
                barrier.subresourceRange.setLevelCount(1);
                barrier.subresourceRange.setBaseArrayLayer(groupStart);
                barrier.subresourceRange.setLayerCount(groupLength);

                groupLength = 0;
            };

            for (uint32_t i = 0; i < touchedLayerCount; i++) {
                uint32_t layerIndex = firstLayer + i;
                VulkanAttachmentState& oldState = subresourceStates[layerIndex];

                bool sameAsNeeded = oldState.imageLayout == attachmentState.newState.imageLayout && oldState.access == attachmentState.newState.access && oldState.stage == attachmentState.newState.stage;
                bool sameAsGroup = groupLength > 0 && oldState.imageLayout == groupOldState.imageLayout && oldState.access == groupOldState.access && oldState.stage == groupOldState.stage;

                if (!sameAsNeeded && sameAsGroup) {
                    groupLength++;
                } else {
                    flushGroup();
                    if (!sameAsNeeded) {
                        groupStart = layerIndex;
                        groupOldState = oldState;
                        groupLength = 1;
                    }
                }

                oldState = attachmentState.newState;
            }

            flushGroup();
        }

        if (!barriers.empty()) {
            vk::DependencyInfo depInfo{};
            depInfo.setImageMemoryBarriers(barriers);
            commandBuffer.pipelineBarrier2(depInfo);
        }

        attachmentStates.clear();
    }
}
