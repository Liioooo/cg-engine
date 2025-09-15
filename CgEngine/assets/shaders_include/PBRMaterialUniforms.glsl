#ifndef PBR_MATERIAL_UNIFORMS_GLSL
#define PBR_MATERIAL_UNIFORMS_GLSL

#include "Macros.glsl"

layout(location = 0) uniform vec3 u_Mat_AlbedoColor;
layout(location = 1) uniform float u_Mat_Metalness;
layout(location = 2) uniform float u_Mat_Roughness;
layout(location = 3) uniform vec3 u_Mat_Emission;
layout(location = 4) uniform bool u_Mat_UseNormals;

PUSH_CONSTANT(MaterialPushConstants, 11) {
    vec3 albedoColor;
    float metalness;
    float roughness;
    vec3 emission;
    bool useNormals;
} pc_material;

layout(binding = 0) uniform sampler2D u_Mat_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_Mat_NormalTexture;
layout(binding = 2) uniform sampler2D u_Mat_MetalnessTexture;
layout(binding = 3) uniform sampler2D u_Mat_RoughnessTexture;
layout(binding = 4) uniform sampler2D u_Mat_EmissionTexture;

#endif // PBR_MATERIAL_UNIFORMS_GLSL
