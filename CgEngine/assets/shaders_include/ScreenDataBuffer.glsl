#ifndef SCREEN_DATA_BUFFER_GLSL
#define SCREEN_DATA_BUFFER_GLSL

#include "Macros.glsl"

UNIFORM_LAYOUT_STD140(3, 0) uniform ScreenData {
    vec2 invFullResolution;
    vec2 fullResolution;
    vec2 invHalfResolution;
    vec2 halfResolution;
} u_ScreenData;

#endif // SCREEN_DATA_BUFFER_GLSL
