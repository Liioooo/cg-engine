#include "common/LightDataBuffer.glsl"
#include "common/LightCalculationsHelperFragment.glsl"

vec3 calcSpotLights(vec3 F0, vec3 matAlbedo, float matMetalness, float matRoughness, vec3 worldPos, vec3 N, vec3 V, float NdotV) {
    vec3 result = vec3(0.0f);

    for (int i = 0; i < u_LightData.spotLightCount; i++) {
        SpotLight light = u_LightData.spotLights[i];

        vec3 L = normalize(light.position.xyz - worldPos);
        float lightDistance = length(light.position.xyz - worldPos);
        vec3 H = normalize(L + V);

        float attenuation = squareDistanceAttenuation(lightDistance, light.radius, light.falloff) * spotAngleAttenuation(L, light.direction.xyz, light.innerAngle, light.outerAngle);

        vec3 Lradiance = light.color.xyz * light.intensity * attenuation;

        float NdotL = max(dot(N, L), 0.0f);

        vec3 F = fresnelSchlick(F0, max(dot(H, V), 0.0f));
        float D = distributionGGX(N, H, matRoughness);
        float G = geometrySmith(NdotL, NdotV, matRoughness);

        vec3 kd = (1.0f - F) * (1.0f - matMetalness);
        vec3 diffuseBRDF = kd * matAlbedo;
        vec3 specularBRDF = (F * D * G) / max(0.0001, 4.0 * NdotL * NdotV);
        specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
        result += (diffuseBRDF + specularBRDF) * Lradiance * NdotL;
    }

    return result;
}
