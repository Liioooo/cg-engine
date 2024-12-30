#pragma once

#include "Macros.h"
#include "Logging.h"
#include <glad/glad.h>

namespace CgEngine {

    struct OpenGLDebugGroup {

        explicit OpenGLDebugGroup(const std::string& label) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, eventId, -1, label.c_str());
        };

        ~OpenGLDebugGroup() {
            glPopDebugGroup();
        }

        static inline unsigned int eventId = 0;
    };
}

#if defined(CG_ENABLE_DEBUG_FEATURES)
    #define CG_GPU_DEBUG_GROUP(label) CgEngine::OpenGLDebugGroup __cg_gpu_debug_group__(label);
#else
    #define CG_GPU_DEBUG_GROUP(label)
#endif


