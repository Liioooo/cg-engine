#pragma once
#include "VulkanAttachment.h"
#include "VulkanVertexBuffer.h"

namespace CgEngine {
    class VulkanBarrierManager {
    public:
        VulkanBarrierManager() = default;

        void requestAttachmentState(VulkanAttachment* attachment, VulkanAttachmentState newState, bool allLayers = true, uint32_t layer = ~0);
        void requestGpuDynamicVertexBufferState(VulkanVertexBuffer* buffer, VulkanGPUDynamicVertexBufferState newState);

        void flushBarriers(vk::CommandBuffer commandBuffer);

    private:

        struct RequestedState {
            VulkanAttachment* attachment;
            VulkanAttachmentState newState;
            bool allLayers;
            uint32_t layer;
        };

        struct RequestedBufferState {
            VulkanVertexBuffer* buffer;
            VulkanGPUDynamicVertexBufferState newState;
        };

        std::vector<RequestedState> attachmentStates;
        std::vector<RequestedBufferState> bufferStates;
    };
}
