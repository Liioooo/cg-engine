#pragma once

#include "Macros.h"
#include "Logging.h"
#include <glad/glad.h>

namespace CgEngine {

    struct OpenGLTimer {
        std::string label;
        bool insertDebugGroup;
        unsigned int queryID[2];

        OpenGLTimer(std::string label, bool insertDebugGroup = false) : label(std::move(label)), insertDebugGroup(insertDebugGroup) {
            if (insertDebugGroup) {
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, this->label.c_str());
            }
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

            if (insertDebugGroup) {
                glPopDebugGroup();
            }
            CG_LOGGING_DEBUG("{0}: GPU TIME: {1}ms", label, (stopTime - startTime) / 1000000.0f)
        }
    };
}

// #define CG_ENABLE_GPU_TIMERS

#if defined(CG_ENABLE_DEBUG_FEATURES) && defined(CG_ENABLE_GPU_TIMERS)
    #define CG_GPU_TIME_FN_ALWAYS() OpenGLTimer __cg_gpu_timer__("Timer: " + std::string(__FUNCTION__));
    #define CG_GPU_TIME_FN(enable, debugGroup) if (enable) { OpenGLTimer __cg_gpu_timer__("Timer: " + std::string(__FUNCTION__), debugGroup); }
    #define CG_GPU_TIME_FN_INFO(label, debugGroup, enable) if (enable) { OpenGLTimer __cg_gpu_timer__("Timer: " + std::string(__FUNCTION__) + ", INFO: " + label, debugGroup); }
#else
    #define CG_GPU_TIME_FN_ALWAYS()
    #define CG_GPU_TIME_FN(enable, debugGroup)
    #define CG_GPU_TIME_FN_INFO(label)
#endif


