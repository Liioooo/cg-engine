#ifndef DIR_SHADOW_MAPPING_GLSL
#define DIR_SHADOW_MAPPING_GLSL

layout (binding = 2, std140) uniform DirShadowData {
    mat4 lightSpaceMat[4];
    vec4 cascadeSplits;
} u_DirShadowData;

layout(binding = 8) uniform sampler2DArray u_DirShadowMap;

vec4[4] calcDirShadowMapPostion(vec3 worldPosition) {
    vec4[4] dirShadowMapPosition;

    dirShadowMapPosition[0] = u_DirShadowData.lightSpaceMat[0] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[1] = u_DirShadowData.lightSpaceMat[1] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[2] = u_DirShadowData.lightSpaceMat[2] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[3] = u_DirShadowData.lightSpaceMat[3] * vec4(worldPosition, 1.0f);

    return dirShadowMapPosition;
}

int getShadowCascade(float depth) {
    for (int i = 0; i < 4; i++) {
        if (depth < u_DirShadowData.cascadeSplits[i]) {
            return i;
        }
    }
    return 3;
}

float sampleShadowMap(int layer, float NdotL, vec4 dirShadowMapPosition[4]) {
    vec3 projCoords = dirShadowMapPosition[layer].xyz / dirShadowMapPosition[layer].w;
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

float calcDirShadow(vec3 N, vec3 L, mat4 viewMatrix, vec3 worldPosition, vec4 dirShadowMapPosition[4]) {
    vec4 viewSpacePos = viewMatrix * vec4(worldPosition, 1.0f);
    float depth = abs(viewSpacePos.z);

    float c0 = smoothstep(u_DirShadowData.cascadeSplits[0] - 1.2f, u_DirShadowData.cascadeSplits[0] + 1.2f, depth);
    float c1 = smoothstep(u_DirShadowData.cascadeSplits[1] - 1.2f, u_DirShadowData.cascadeSplits[1] + 1.2f, depth);
    float c2 = smoothstep(u_DirShadowData.cascadeSplits[2] - 1.2f, u_DirShadowData.cascadeSplits[2] + 1.2f, depth);

    float NdotL = dot(N, L);

    if (c0 > 0.0 && c0 < 1.0) {
        float f0 = sampleShadowMap(0, NdotL, dirShadowMapPosition);
        float f1 = sampleShadowMap(1, NdotL, dirShadowMapPosition);
        return mix(f0, f1, c0);
    } else if (c1 > 0.0 && c1 < 1.0) {
        float f1 = sampleShadowMap(1, NdotL, dirShadowMapPosition);
        float f2 = sampleShadowMap(2, NdotL, dirShadowMapPosition);
        return mix(f1, f2, c1);
    } else if (c2 > 0.0 && c2 < 1.0) {
        float f2 = sampleShadowMap(2, NdotL, dirShadowMapPosition);
        float f3 = sampleShadowMap(3, NdotL, dirShadowMapPosition);
        return mix(f2, f3, c2);
    } else {
        return sampleShadowMap(getShadowCascade(depth), NdotL, dirShadowMapPosition);
    }
}

#endif // DIR_SHADOW_MAPPING_GLSL
