#pragma once

#include "Macros.h"
#include "Logging.h"
#include <glad/glad.h>

namespace CgEngine {

    struct OpenGLTimer {
        std::string label;
        float* timeOutput;
        bool printToConsole;
        unsigned int queryID[2];

        OpenGLTimer(std::string label, float* timeOutput = nullptr, bool printToConsole = false) : label(std::move(label)), timeOutput(timeOutput), printToConsole(printToConsole) {
            glGenQueries(2, queryID);
            glQueryCounter(queryID[0], GL_TIMESTAMP);
        };

        ~OpenGLTimer() {
            glQueryCounter(queryID[1], GL_TIMESTAMP);

            int available = 0;
            while (!available) {
                glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &available);
            }

            unsigned long long startTime, stopTime;
            glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &startTime);
            glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stopTime);

            glDeleteQueries(2, queryID);

            float time = (stopTime - startTime) / 1000000.0f;

            if (timeOutput != nullptr) {
                *timeOutput = time;
            }

            if (printToConsole) {
                CG_LOGGING_DEBUG("{0}: GPU TIME: {1}ms", label, time)
            }
        }
    };
}

#if defined(CG_ENABLE_DEBUG_FEATURES)
    #define CG_GPU_TIME_FN(timeOutput, printToConsole) OpenGLTimer __cg_gpu_timer__("Timer: " + std::string(__FUNCTION__), timeOutput, printToConsole);
#else
    #define CG_GPU_TIME_FN(timeOutput, printToConsole)
#endif


