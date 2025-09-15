#ifndef SCREEN_DATA_BUFFER_GLSL
#define SCREEN_DATA_BUFFER_GLSL

layout(std140, binding = 3) uniform ScreenData {
    vec2 invFullResolution;
    vec2 fullResolution;
    vec2 invHalfResolution;
    vec2 halfResolution;
} u_ScreenData;

#endif // SCREEN_DATA_BUFFER_GLSL
