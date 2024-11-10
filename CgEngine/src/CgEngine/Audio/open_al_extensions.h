#pragma once

#include <alc.h>
#include <alext.h>

inline auto alDebugMessageCallbackEXT = LPALDEBUGMESSAGECALLBACKEXT{};
inline auto alDebugMessageInsertEXT = LPALDEBUGMESSAGEINSERTEXT{};
inline auto alDebugMessageControlEXT = LPALDEBUGMESSAGECONTROLEXT{};
inline auto alPushDebugGroupEXT = LPALPUSHDEBUGGROUPEXT{};
inline auto alPopDebugGroupEXT = LPALPOPDEBUGGROUPEXT{};
inline auto alGetDebugMessageLogEXT = LPALGETDEBUGMESSAGELOGEXT{};
inline auto alObjectLabelEXT = LPALOBJECTLABELEXT{};
inline auto alGetObjectLabelEXT = LPALGETOBJECTLABELEXT{};
inline auto alGetPointerEXT = LPALGETPOINTEREXT{};
inline auto alGetPointervEXT = LPALGETPOINTERVEXT{};

namespace CgEngine::OpenAlExtensions {
    inline bool has_ALC_EXT_debug = false;
    inline bool has_AL_SOFT_events = false;

    void loadOpenAlExtensions(ALCdevice* device) {
        has_ALC_EXT_debug = alcIsExtensionPresent(device, "ALC_EXT_debug");

        if (has_ALC_EXT_debug) {
            #define LOAD_PROC(N) N = reinterpret_cast<decltype(N)>(alcGetProcAddress(device, #N))
                LOAD_PROC(alDebugMessageCallbackEXT);
                LOAD_PROC(alDebugMessageInsertEXT);
                LOAD_PROC(alDebugMessageControlEXT);
                LOAD_PROC(alPushDebugGroupEXT);
                LOAD_PROC(alPopDebugGroupEXT);
                LOAD_PROC(alGetDebugMessageLogEXT);
                LOAD_PROC(alObjectLabelEXT);
                LOAD_PROC(alGetObjectLabelEXT);
                LOAD_PROC(alGetPointerEXT);
                LOAD_PROC(alGetPointervEXT);
            #undef LOAD_PROC
        }
    }
}
