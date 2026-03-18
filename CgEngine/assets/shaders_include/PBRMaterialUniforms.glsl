#ifndef PBR_MATERIAL_UNIFORMS_GLSL
#define PBR_MATERIAL_UNIFORMS_GLSL

layout (binding = 5, std140) uniform PBRMaterialData {
    vec3 albedoColor;
    float metalness;
    float roughness;
    vec3 emission;
    bool useNormals;
} u_PBRMaterialData;

layout(binding = 0) uniform sampler2D u_Mat_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_Mat_NormalTexture;
layout(binding = 2) uniform sampler2D u_Mat_MetalnessTexture;
layout(binding = 3) uniform sampler2D u_Mat_RoughnessTexture;
layout(binding = 4) uniform sampler2D u_Mat_EmissionTexture;

#endif // PBR_MATERIAL_UNIFORMS_GLSL
