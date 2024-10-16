#version 450 core

#include "common/PBRMaterialUniforms.glsl"
#include "common/CameraDataBuffer.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/DirShadowMappingFragment.glsl"
#include "common/IBLCalculationsFragment.glsl"
#include "common/LightCalculationsHelperFragment.glsl"

layout(early_fragment_tests) in;

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    mat3 TBN;
    vec2 TexCoord;
} fs_in;

out vec4 o_FragColor;

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
    vec3 specularBRDF = (F * D * G) / max(0.0001, 4.0 * NdotL * NdotV);
    specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
    return (diffuseBRDF + specularBRDF) * Lradiance * NdotL * (1.0f - calcDirShadow(N, L, u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
}

vec3 calcPointLights(vec3 F0, vec3 matAlbedo, float matMetalness, float matRoughness, vec3 worldPos, vec3 N, vec3 V, float NdotV) {
    vec3 result = vec3(0.0f);

    for (int i = 0; i < u_LightData.pointLightCount; i++) {
        PointLight light = u_LightData.pointLights[i];

        vec3 L = normalize(light.position.xyz - worldPos);
        float lightDistance = length(light.position.xyz - worldPos);
        vec3 H = normalize(L + V);

        float attenuation = squareDistanceAttenuation(lightDistance, light.radius, light.falloff);

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

void main() {
    vec3 mat_AlbedoColor = texture(u_Mat_AlbedoTexture, fs_in.TexCoord).rgb * u_Mat_AlbedoColor;
    float mat_Metalness = texture(u_Mat_MetalnessTexture, fs_in.TexCoord).r * u_Mat_Metalness;
    float mat_Roughness = texture(u_Mat_RoughnessTexture, fs_in.TexCoord).r * u_Mat_Roughness;
    vec3 mat_Emission = texture(u_Mat_EmissionTexture, fs_in.TexCoord).rgb * u_Mat_Emission;

    vec3 mat_Normal = normalize(fs_in.Normal);
    if (u_Mat_UseNormals) {
        mat_Normal = normalize(texture(u_Mat_NormalTexture, fs_in.TexCoord).rgb * 2.0f - 1.0f);
        mat_Normal = normalize(fs_in.TBN * mat_Normal);
    }

    vec3 V = normalize(u_CameraData.position.xyz - fs_in.WorldPosition);
    float NdotV = max(dot(mat_Normal, V), 0.0f);

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, mat_AlbedoColor, mat_Metalness);

    vec3 light = calcDirLight(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV);
    light += calcPointLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, fs_in.WorldPosition, mat_Normal, V, NdotV);
    light += calcSpotLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, fs_in.WorldPosition, mat_Normal, V, NdotV);
    light += mat_Emission;

    vec3 ibl = calcIBL(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * u_EnvironmentIntensity;

    o_FragColor = vec4(light + ibl, 1.0f);


//    vec4 viewSpacePos = u_CameraData.view * vec4(fs_in.WorldPosition, 1.0f);
//    switch (getShadowCascade(abs(viewSpacePos.z)))
//    {
//        case 0:
//        o_FragColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
//        break;
//        case 1:
//        o_FragColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
//        break;
//        case 2:
//        o_FragColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
//        break;
//        case 3:
//        o_FragColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
//        break;
//    }
}
