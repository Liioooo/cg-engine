#pragma once

#include "Logging.h"

namespace CgEngine {

    struct Timer {
        std::string label;
        std::chrono::time_point<std::chrono::system_clock> start, end;
        std::chrono::duration<float> duration;

        Timer(std::string label) : label(std::move(label)) {
            start = std::chrono::high_resolution_clock::now();
        };

        ~Timer() {
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;

            float ms = duration.count() * 1000.0f;
            CG_LOGGING_DEBUG("{0}: {1}ms", label, ms)
        }
    };
}

#ifdef CG_ENABLE_DEBUG_FEATURES
    #define CG_TIME_FN() Timer __cg_timer__("Timer: " + std::string(__FUNCTION__));
    #define CG_TIME_FN_INFO(label) Timer __cg_timer__("Timer: " + std::string(__FUNCTION__) + ", INFO: " + label);
#else
    #define CG_TIME_FN()
    #define CG_TIME_FN_INFO(label)
#endif

