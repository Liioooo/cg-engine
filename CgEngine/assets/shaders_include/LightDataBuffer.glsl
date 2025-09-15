#ifndef LIGHT_DATA_BUFFER_GLSL
#define LIGHT_DATA_BUFFER_GLSL

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float radius;
    float falloff;
};

struct SpotLight {
    vec4 position;
    vec4 color;
    vec4 direction;
    float intensity;
    float radius;
    float falloff;
    float innerAngle;
    float outerAngle;
};

layout (binding = 1, std140) uniform LightData {
    vec4 dirLightDirection;
    vec4 dirLightColor;
    float dirLightIntensity;
    int pointLightCount;
    int spotLightCount;
    PointLight pointLights[100];
    SpotLight spotLights[100];
} u_LightData;

#endif // LIGHT_DATA_BUFFER_GLSL
