#ifndef IBL_CALCULATIONS_FRAGMENT_GLSL
#define IBL_CALCULATIONS_FRAGMENT_GLSL

#include "LightCalculationsHelperFragment.glsl"

UNIFORM_LAYOUT(5, 1) uniform samplerCube u_IrradianceMap;
UNIFORM_LAYOUT(6, 1) uniform samplerCube u_PrefilterMap;
UNIFORM_LAYOUT(7, 0) uniform sampler2D u_BrdfLUT;

vec3 calcIBL(vec3 F0, vec3 matAlbedo, float matMetalness, float matRoughness, vec3 N, vec3 V, float NdotV) {
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 F = fresnelSchlick(F0, NdotV);
    vec3 kd = (1.0f - F) * (1.0f - matMetalness);
    vec3 diffuseIBL = matAlbedo * irradiance;

    vec3 R = reflect(-V, N);
    int envPrefilterTexLevels = textureQueryLevels(u_PrefilterMap);
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, matRoughness * envPrefilterTexLevels).rgb;
    vec2 specularBRDF = texture(u_BrdfLUT, vec2(NdotV, 1.0f - matRoughness)).rg;
    vec3 specularIBL = prefilteredColor * (F0 * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

#endif // IBL_CALCULATIONS_FRAGMENT_GLSL
