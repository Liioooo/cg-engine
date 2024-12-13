#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/DirShadowMapping.glsl"
#include "common/DirLightCalculationsFragment.glsl"
#include "common/PointLightsCalculationsFragment.glsl"
#include "common/SpotLightsCalculationsFragment.glsl"
#include "common/IBLCalculationsFragment.glsl"
#include "common/LightCalculationsHelperFragment.glsl"
#include "common/HBAOSampling.glsl"

layout(binding = 0) uniform sampler2D u_gBuffer_AlbedoRoughness;
layout(binding = 1) uniform sampler2D u_gBuffer_EmissionMetallic;
layout(binding = 2) uniform sampler2D u_gBuffer_WorldNormal;
layout(binding = 3) uniform sampler2D u_Depth;

in VS_OUT {
    vec2 TexCoord;
} fs_in;

out vec4 o_FragColor;

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

    vec3 ibl = calcIBL(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * u_EnvironmentIntensity * texture(u_HBAO_Tex, fs_in.TexCoord).r;

    o_FragColor = vec4(light + ibl, 1.0f);
}
