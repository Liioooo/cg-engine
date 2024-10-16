#include "common/LightCalculationsHelperFragment.glsl"

uniform float u_EnvironmentIntensity;
layout(binding = 5) uniform samplerCube u_IrradianceMap;
layout(binding = 6) uniform samplerCube u_PrefilterMap;
layout(binding = 7) uniform sampler2D u_BrdfLUT;

vec3 calcIBL(vec3 F0, vec3 matAlbedo, float matMetalness, float matRoughness, vec3 N, vec3 V, float NdotV) {
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 F = fresnelSchlick(F0, NdotV);
    vec3 kd = (1.0f - F) * (1.0f - matMetalness);
    vec3 diffuseIBL = matAlbedo * irradiance;

    vec3 R = reflect(-V, N);
    int envPrefilterTexLevels = textureQueryLevels(u_PrefilterMap);
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R,  matRoughness * envPrefilterTexLevels).rgb;
    vec2 specularBRDF = texture(u_BrdfLUT, vec2(NdotV, matRoughness)).rg;
    vec3 specularIBL = prefilteredColor * (F0 * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}
