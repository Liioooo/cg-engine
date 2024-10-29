layout(std140, binding = 3) uniform ScreenData {
    vec2 invFullResolution;
    vec2 fullResolution;
    vec2 invHalfResolution;
    vec2 halfResolution;
} u_ScreenData;
