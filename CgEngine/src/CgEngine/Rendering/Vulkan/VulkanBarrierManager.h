#pragma once
#include "VulkanAttachment.h"

namespace CgEngine {
    class VulkanBarrierManager {
    public:
        VulkanBarrierManager() = default;

        void requestAttachmentState(VulkanAttachment* attachment, VulkanAttachmentState newState, bool allLayers = true, uint32_t layer = ~0);
        void flushBarriers(vk::CommandBuffer commandBuffer);

    private:

        struct RequestedState {
            VulkanAttachment* attachment;
            VulkanAttachmentState newState;
            bool allLayers;
            uint32_t layer;
        };

        std::vector<RequestedState> attachmentStates;
    };
}
