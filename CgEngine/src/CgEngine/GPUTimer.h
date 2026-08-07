#pragma once

#include "Asserts.h"
#include "Macros.h"
#include <glad/glad.h>
#include "Rendering/GraphicsObjectsFactory.h"
#include "Rendering/Renderer.h"
#include "Rendering/Vulkan/VulkanRenderer.h"

namespace CgEngine {

    struct OpenGLTimerQueryData {
        float* timeOutput;
        unsigned int queryID[2];

        OpenGLTimerQueryData(float* timeOutput) : timeOutput(timeOutput) {}
    };

    struct OpenGLTimer {
        size_t timerIndex = std::numeric_limits<size_t>::max();

        OpenGLTimer() = default;
        OpenGLTimer(float* timeOutput) {
            timerIndex = timerQueries.size();
            auto& data = timerQueries.emplace_back(timeOutput);

            glGenQueries(2, data.queryID);
            glQueryCounter(data.queryID[0], GL_TIMESTAMP);
        };

        ~OpenGLTimer() {
            if (timerIndex != std::numeric_limits<size_t>::max()) {
                glQueryCounter(timerQueries[timerIndex].queryID[1], GL_TIMESTAMP);
            }
        }

        static inline std::vector<OpenGLTimerQueryData> timerQueries;

        static void writeBackQueryResults() {
            for (auto& queryData : timerQueries) {
                int available = 0;
                while (!available) {
                    glGetQueryObjectiv(queryData.queryID[1], GL_QUERY_RESULT_AVAILABLE, &available);
                }

                GLuint64 startTime, stopTime;
                glGetQueryObjectui64v(queryData.queryID[0], GL_QUERY_RESULT, &startTime);
                glGetQueryObjectui64v(queryData.queryID[1], GL_QUERY_RESULT, &stopTime);

                glDeleteQueries(2, queryData.queryID);

                float time = static_cast<float>(stopTime - startTime) / 1000000.0f;
                *queryData.timeOutput = time;
            }

            timerQueries.clear();
        }
    };

    struct VulkanTimerQueryData {
        float* timeOutput;
        uint32_t queryPairIndex;
    };

    // Timestamps are written into the current frame-in-flight's command buffer, so they can't be
    // read back in the same frame they're recorded in (the command buffer isn't submitted yet).
    // Instead, each frame-in-flight slot's results are read back right before that slot's query
    // pool is reused - at that point VulkanRenderer::beginFrame has already waited on the slot's
    // fence, so the GPU is guaranteed to be done with the previous submission that wrote them.
    struct VulkanTimer {
        static constexpr uint32_t MAX_QUERIES_PER_FRAME = 64;

        size_t timerIndex = std::numeric_limits<size_t>::max();
        uint32_t frameSlot = 0;

        VulkanTimer() = default;
        VulkanTimer(float* timeOutput) {
            auto* vulkanRenderer = Renderer::getVulkanBackend();
            vk::Device device = vulkanRenderer->getVkDevice();
            vk::CommandBuffer commandBuffer = vulkanRenderer->getCurrentCommandBuffer();
            uint32_t currentSlot = vulkanRenderer->getCurrentFrameIndex();

            ensureQueryPools(vulkanRenderer->getMaxFramesInFlight(), device);

            if (currentSlot != lastFrameSlot) {
                writeBackFrameResults(currentSlot, device);
                commandBuffer.resetQueryPool(queryPools[currentSlot], 0, MAX_QUERIES_PER_FRAME);
                nextQueryPairIndex[currentSlot] = 0;
                lastFrameSlot = currentSlot;
            }

            CG_ASSERT(nextQueryPairIndex[currentSlot] < MAX_QUERIES_PER_FRAME / 2, "VulkanTimer: exceeded the maximum number of GPU timers in a single frame!")

            frameSlot = currentSlot;
            timerIndex = nextQueryPairIndex[currentSlot]++;
            pendingQueries[currentSlot].push_back({timeOutput, static_cast<uint32_t>(timerIndex)});

            commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eTopOfPipe, queryPools[currentSlot], static_cast<uint32_t>(timerIndex) * 2);
        };

        ~VulkanTimer() {
            if (timerIndex != std::numeric_limits<size_t>::max()) {
                auto* vulkanRenderer = Renderer::getVulkanBackend();
                vulkanRenderer->getCurrentCommandBuffer().writeTimestamp2(vk::PipelineStageFlagBits2::eBottomOfPipe, queryPools[frameSlot], static_cast<uint32_t>(timerIndex) * 2 + 1);
            }
        }

        static inline std::vector<vk::QueryPool> queryPools;
        static inline std::vector<uint32_t> nextQueryPairIndex;
        static inline std::vector<std::vector<VulkanTimerQueryData>> pendingQueries;
        static inline uint32_t lastFrameSlot = std::numeric_limits<uint32_t>::max();

        static void ensureQueryPools(uint32_t maxFramesInFlight, vk::Device device) {
            if (!queryPools.empty()) {
                return;
            }

            queryPools.resize(maxFramesInFlight);
            nextQueryPairIndex.resize(maxFramesInFlight, 0);
            pendingQueries.resize(maxFramesInFlight);

            vk::QueryPoolCreateInfo poolInfo{};
            poolInfo.setQueryType(vk::QueryType::eTimestamp);
            poolInfo.setQueryCount(MAX_QUERIES_PER_FRAME);

            for (auto& pool : queryPools) {
                auto result = device.createQueryPool(poolInfo);
                CG_ASSERT(result.has_value(), "VulkanTimer: Failed to create query pool!")
                pool = result.value;
            }
        }

        static void writeBackFrameResults(uint32_t slot, vk::Device device) {
            auto& pending = pendingQueries[slot];
            if (pending.empty()) {
                return;
            }

            float timestampPeriod = Renderer::getVulkanBackend()->getVkPhysicalDevice().getProperties().limits.timestampPeriod;

            for (auto& query : pending) {
                uint64_t timestamps[2];
                auto result = device.getQueryPoolResults(queryPools[slot], query.queryPairIndex * 2, 2, sizeof(timestamps), timestamps, sizeof(uint64_t), vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
                CG_ASSERT(result == vk::Result::eSuccess, "VulkanTimer: Failed to get query pool results!")

                *query.timeOutput = static_cast<float>(timestamps[1] - timestamps[0]) * timestampPeriod / 1000000.0f;
            }

            pending.clear();
        }

        static void writeBackQueryResults() {
            // no-op: results are written back lazily above, right before each frame-in-flight
            // slot's query pool is reused, since this frame's own queries aren't submitted yet.
        }
    };

    struct GPUTimer {
        OpenGLTimer glTimer;
        VulkanTimer vkTimer;

        GPUTimer(float* timeOutput)
            : glTimer(GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::OpenGL ? OpenGLTimer(timeOutput) : OpenGLTimer()),
              vkTimer(GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::Vulkan ? VulkanTimer(timeOutput) : VulkanTimer()) {}

        static void writeBackQueryResults() {
            if (GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::OpenGL) {
                OpenGLTimer::writeBackQueryResults();
            } else {
                VulkanTimer::writeBackQueryResults();
            }
        }
    };
}

#if defined(CG_ENABLE_DEBUG_FEATURES)
    #define CG_GPU_TIME_FN(timeOutput) CgEngine::GPUTimer __cg_gpu_timer__(timeOutput);
    #define CG_GPU_TIME_WRITE_RESULTS() CgEngine::GPUTimer::writeBackQueryResults();
#else
    #define CG_GPU_TIME_FN(timeOutput)
    #define CG_GPU_TIME_WRITE_RESULTS()
#endif


