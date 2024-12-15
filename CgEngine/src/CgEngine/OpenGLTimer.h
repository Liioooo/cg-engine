#pragma once

#include "Macros.h"
#include "Logging.h"
#include <glad/glad.h>

namespace CgEngine {

    struct OpenGLTimerQueryData {
        float* timeOutput;
        unsigned int queryID[2];

        OpenGLTimerQueryData(float* timeOutput) : timeOutput(timeOutput) {}
    };

    struct OpenGLTimer {
        size_t timerIndex;

        OpenGLTimer(float* timeOutput) {
            timerIndex = timerQueries.size();
            auto& data = timerQueries.emplace_back(timeOutput);

            glGenQueries(2, data.queryID);
            glQueryCounter(data.queryID[0], GL_TIMESTAMP);
        };

        ~OpenGLTimer() {
            glQueryCounter(timerQueries[timerIndex].queryID[1], GL_TIMESTAMP);
        }

        static inline std::vector<OpenGLTimerQueryData> timerQueries;

        static void writeBackQueryResults() {
            for (auto& queryData : timerQueries) {
                int available = 0;
                while (!available) {
                    glGetQueryObjectiv(queryData.queryID[1], GL_QUERY_RESULT_AVAILABLE, &available);
                }

                unsigned long long startTime, stopTime;
                glGetQueryObjectui64v(queryData.queryID[0], GL_QUERY_RESULT, &startTime);
                glGetQueryObjectui64v(queryData.queryID[1], GL_QUERY_RESULT, &stopTime);

                glDeleteQueries(2, queryData.queryID);

                float time = static_cast<float>(stopTime - startTime) / 1000000.0f;
                *queryData.timeOutput = time;
            }

            timerQueries.clear();
        }
    };
}

#if defined(CG_ENABLE_DEBUG_FEATURES)
    #define CG_GPU_TIME_FN(timeOutput) OpenGLTimer __cg_gpu_timer__(timeOutput);
    #define CG_GPU_TIME_WRITE_RESULTS() OpenGLTimer::writeBackQueryResults();
#else
    #define CG_GPU_TIME_FN(timeOutput)
    #define CG_GPU_TIME_WRITE_RESULTS()
#endif


