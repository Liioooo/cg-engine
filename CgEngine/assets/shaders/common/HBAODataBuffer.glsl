layout(std140, binding = 4) uniform HBAOData {
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
