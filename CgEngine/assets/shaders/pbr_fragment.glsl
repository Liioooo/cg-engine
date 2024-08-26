#version 450 core

#include "common/CameraDataBuffer.glsl"

layout(early_fragment_tests) in;

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float radius;
    float falloff;
};

struct SpotLight {
    vec4 position;
    vec4 color;
    vec4 direction;
    float intensity;
    float radius;
    float falloff;
    float innerAngle;
    float outerAngle;
};

layout (binding = 1, std140) uniform LightData {
    vec4 dirLightDirection;
    vec4 dirLightColor;
    float dirLightIntensity;
    int pointLightCount;
    int spotLightCount;
    PointLight pointLights[100];
    SpotLight spotLights[100];
} u_LightData;

layout (binding = 2, std140) uniform DirShadowData {
    mat4 lightSpaceMat[4];
    vec4 cascadeSplits;
} u_DirShadowData;

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    mat3 TBN;
    vec2 TexCoord;
} fs_in;

out vec4 o_FragColor;

uniform vec3 u_Mat_AlbedoColor;
uniform float u_Mat_Metalness;
uniform float u_Mat_Roughness;
uniform vec3 u_Mat_Emission;
uniform bool u_Mat_UseNormals;
layout(binding = 0) uniform sampler2D u_Mat_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_Mat_NormalTexture;
layout(binding = 2) uniform sampler2D u_Mat_MetalnessTexture;
layout(binding = 3) uniform sampler2D u_Mat_RoughnessTexture;
layout(binding = 4) uniform sampler2D u_Mat_EmissionTexture;

uniform float u_EnvironmentIntensity;
layout(binding = 5) uniform samplerCube u_IrradianceMap;
layout(binding = 6) uniform samplerCube u_PrefilterMap;
layout(binding = 7) uniform sampler2D u_BrdfLUT;

layout(binding = 8) uniform sampler2DArray u_DirShadowMap;

const float PI = 3.141592;

float squareDistanceAttenuation(float distance, float lightRadius, float lightFalloff) {
    float attenuation = clamp(1.0 - (distance * distance) / (lightRadius * lightRadius), 0.0, 1.0);
    return attenuation * mix(attenuation, 1.0, lightFalloff);
}

float spotAngleAttenuation(vec3 L, vec3 lightDir, float innerAngle, float outerAngle) {
    float cosOuter = cos(outerAngle);
    float spotScale = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
    float spotOffset = -cosOuter * spotScale;

    float cd = dot(normalize(-lightDir), L);
    float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
    return attenuation * attenuation;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float NdotH  = max(dot(N, H), 0.0);

    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

int getShadowCascade(float depth) {
    for (int i = 0; i < 4; i++) {
        if (depth < u_DirShadowData.cascadeSplits[i]) {
            return i;
        }
    }
    return 3;
}

float sampleShadowMap(int layer, float NdotL) {
    vec3 projCoords = fs_in.DirShadowMapPosition[layer].xyz / fs_in.DirShadowMapPosition[layer].w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    if (currentDepth > 1.0f) {
        return 0.0f;
    }

    float bias = layer == 0 ? max(0.0001f * (1.0f - NdotL), 0.0002f) : layer == 1 ? max(0.0005f * (1.0f - NdotL), 0.0005f) : max(0.0005f * (1.0f - NdotL), 0.001f);

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / vec2(textureSize(u_DirShadowMap, 0));
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            float pcfDepth = texture(u_DirShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += step(pcfDepth, (currentDepth - bias));
        }
    }
    shadow /= 9.0f;

    return shadow;
}

float calcDirShadow(vec3 N, vec3 L) {
    vec4 viewSpacePos = u_CameraData.view * vec4(fs_in.WorldPosition, 1.0f);
    float depth = abs(viewSpacePos.z);

    float c0 = smoothstep(u_DirShadowData.cascadeSplits[0] - 1.2f, u_DirShadowData.cascadeSplits[0] + 1.2f, depth);
    float c1 = smoothstep(u_DirShadowData.cascadeSplits[1] - 1.2f, u_DirShadowData.cascadeSplits[1] + 1.2f, depth);
    float c2 = smoothstep(u_DirShadowData.cascadeSplits[2] - 1.2f, u_DirShadowData.cascadeSplits[2] + 1.2f, depth);

    float NdotL = dot(N, L);

    if (c0 > 0.0 && c0 < 1.0) {
        float f0 = sampleShadowMap(0, NdotL);
        float f1 = sampleShadowMap(1, NdotL);
        return mix(f0, f1, c0);
    } else if (c1 > 0.0 && c1 < 1.0) {
        float f1 = sampleShadowMap(1, NdotL);
        float f2 = sampleShadowMap(2, NdotL);
        return mix(f1, f2, c1);
    } else if (c2 > 0.0 && c2 < 1.0) {
        float f2 = sampleShadowMap(2, NdotL);
        float f3 = sampleShadowMap(3, NdotL);
        return mix(f2, f3, c2);
    } else {
        return sampleShadowMap(getShadowCascade(depth), NdotL);
    }
}

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
    return (diffuseBRDF + specularBRDF) * Lradiance * NdotL * (1.0f - calcDirShadow(N, L));
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
