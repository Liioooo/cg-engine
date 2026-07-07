#ifndef PBR_MATERIAL_UNIFORMS_GLSL
#define PBR_MATERIAL_UNIFORMS_GLSL

#include "Macros.glsl"

UNIFORM_LAYOUT_STD140(5, 1) uniform PBRMaterialData {
    vec3 albedoColor;
    float metalness;
    float roughness;
    vec3 emission;
    bool useNormals;
} u_PBRMaterialData;

UNIFORM_LAYOUT(0, 1) uniform sampler2D u_Mat_AlbedoTexture;
UNIFORM_LAYOUT(1, 1) uniform sampler2D u_Mat_NormalTexture;
UNIFORM_LAYOUT(2, 1) uniform sampler2D u_Mat_MetalnessTexture;
UNIFORM_LAYOUT(3, 1) uniform sampler2D u_Mat_RoughnessTexture;
UNIFORM_LAYOUT(4, 1) uniform sampler2D u_Mat_EmissionTexture;

#endif // PBR_MATERIAL_UNIFORMS_GLSL
