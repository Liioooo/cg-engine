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


    struct VulkanTimer {

        VulkanTimer() = default;
        VulkanTimer(float* timeOutput) {};

        ~VulkanTimer() {
        }

        static void writeBackQueryResults() {
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


