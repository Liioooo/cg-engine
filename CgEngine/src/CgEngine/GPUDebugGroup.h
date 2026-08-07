#pragma once

#include "Macros.h"
#include <glad/glad.h>

#include "Rendering/GraphicsObjectsFactory.h"

namespace CgEngine {

    struct OpenGLDebugGroup {
        bool pushedGroup = false;

        OpenGLDebugGroup() = default;

        OpenGLDebugGroup(const OpenGLDebugGroup&) = delete;
        OpenGLDebugGroup& operator=(const OpenGLDebugGroup&) = delete;
        OpenGLDebugGroup(OpenGLDebugGroup&&) = delete;
        OpenGLDebugGroup& operator=(OpenGLDebugGroup&&) = delete;

        ~OpenGLDebugGroup() {
            if (pushedGroup) {
                glPopDebugGroup();
            }
        }

        void push(const std::string& label) {
            pushedGroup = true;
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, eventId, -1, label.c_str());
        }

        static inline unsigned int eventId = 0;
    };

    struct GPUDebugGroup {
        OpenGLDebugGroup glDebugGroup;

        explicit GPUDebugGroup(const std::string& label) {
            if (GraphicsObjectsFactory::getGraphicsAPI() == GraphicsAPI::OpenGL) {
                glDebugGroup.push(label);
            }
        }
    };
}

#if defined(CG_ENABLE_DEBUG_FEATURES)
    #define CG_GPU_DEBUG_GROUP(label) CgEngine::GPUDebugGroup __cg_gpu_debug_group__(label);
#else
    #define CG_GPU_DEBUG_GROUP(label)
#endif


