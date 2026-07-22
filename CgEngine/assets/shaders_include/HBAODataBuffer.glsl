#ifndef HBAO_DATA_BUFFER_GLSL
#define HBAO_DATA_BUFFER_GLSL

#include "Macros.glsl"

UNIFORM_LAYOUT_STD140(4, 0) uniform HBAOData {
    vec4 perspectiveInfo;
    vec2 invQuarterResolution;
    float radiusToScreen;
    float negInvR2;
    float nDotVBias;
    float aoMultiplier;
    float powExponent;
    bool isOrtho;
    vec4 float2Offsets[16];
    vec4 jitters[16];
} u_HBAO;

#endif // HBAO_DATA_BUFFER_GLSL
