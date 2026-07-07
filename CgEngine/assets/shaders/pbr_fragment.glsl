#version 450 core

#include "CameraDataBuffer.glsl"
#include "LightDataBuffer.glsl"
#include "DirShadowMapping.glsl"
#include "DirLightCalculationsFragment.glsl"
#include "PointLightsCalculationsFragment.glsl"
#include "SpotLightsCalculationsFragment.glsl"
#include "IBLCalculationsFragment.glsl"
#include "LightCalculationsHelperFragment.glsl"
#include "Macros.glsl"

UNIFORM_LAYOUT(9, 0)  uniform sampler2D u_HBAO_Tex;
UNIFORM_LAYOUT(10, 0) uniform sampler2D u_gBuffer_AlbedoRoughness;
UNIFORM_LAYOUT(11, 0) uniform sampler2D u_gBuffer_EmissionMetallic;
UNIFORM_LAYOUT(12, 0) uniform sampler2D u_gBuffer_WorldNormal;
UNIFORM_LAYOUT(13, 0) uniform sampler2D u_Depth;

PUSH_CONSTANT(PbrPC) {
    float environmentIntensity;
} pc_pbr;

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout(location = 0) out vec4 o_FragColor;

void main() {
    vec4 albedoRoughnessSample = texture(u_gBuffer_AlbedoRoughness, fs_in.TexCoord);
    vec4 emissionMetallicSample = texture(u_gBuffer_EmissionMetallic, fs_in.TexCoord);

    vec3 mat_AlbedoColor = albedoRoughnessSample.rgb;
    float mat_Roughness = albedoRoughnessSample.a;
    vec3 mat_Emission = emissionMetallicSample.rgb;
    float mat_Metalness = emissionMetallicSample.a;
    vec3 mat_Normal = texture(u_gBuffer_WorldNormal, fs_in.TexCoord).xyz;

    vec2 ndc = fs_in.TexCoord * 2.0f - 1.0f;
    float screenDepth = texture(u_Depth, fs_in.TexCoord).r;
    vec4 clipSpacePos = vec4(ndc, screenDepth * 2.0f - 1.0f, 1.0f);
    vec4 worldSpacePos = u_CameraData.invViewProjection * clipSpacePos;
    vec3 worldPosition = worldSpacePos.xyz / worldSpacePos.w;

    vec3 V = normalize(u_CameraData.position.xyz - worldPosition);
    float NdotV = max(dot(mat_Normal, V), 0.0f);

    vec3 F0 = calcF0(mat_AlbedoColor, mat_Metalness);

    vec3 light = calcDirLight(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * (1.0f - calcDirShadow(mat_Normal, u_LightData.dirLightDirection.xyz, u_CameraData.view, worldPosition, calcDirShadowMapPostion(worldPosition)));
    light += calcPointLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, worldPosition, mat_Normal, V, NdotV);
    light += calcSpotLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, worldPosition, mat_Normal, V, NdotV);
    light += mat_Emission;

    vec3 ibl = calcIBL(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * pc_pbr.environmentIntensity * texture(u_HBAO_Tex, fs_in.TexCoord).r;

    o_FragColor = vec4(light + ibl, 1.0f);
}
