#include "common/LightDataBuffer.glsl"
#include "common/LightCalculationsHelperFragment.glsl"

vec3 calcDirLight(vec3 F0, vec3 matAlbedo, float matMetalness, float matRoughness, vec3 N, vec3 V, float NdotV) {
    if (u_LightData.dirLightIntensity == 0.0f) {
        return vec3(0.0f);
    }

    vec3 L = u_LightData.dirLightDirection.xyz;
    vec3 Lradiance =  u_LightData.dirLightColor.xyz * u_LightData.dirLightIntensity;
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0f);

    vec3 F = fresnelSchlick(F0, max(dot(H, V), 0.0f));
    float D = distributionGGX(N, H, matRoughness);
    float G = geometrySmith(NdotL, NdotV, matRoughness);

    vec3 kd = (1.0f - F) * (1.0f - matMetalness);
    vec3 diffuseBRDF = kd * matAlbedo;
    vec3 specularBRDF = (F * D * G) / max(0.0001f, 4.0f * NdotL * NdotV);
    specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
    return (diffuseBRDF + specularBRDF) * Lradiance * NdotL;
}
