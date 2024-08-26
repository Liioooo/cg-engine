#version 450 core

layout(binding = 0) uniform samplerCube u_cubeMap;
layout(binding = 1, rgba32f) restrict writeonly uniform imageCube o_prefilterMap;

uniform float u_Roughness;

const uint SAMPLE_COUNT = 1024u;
const float INVERSE_SAMPLE_COUNT = 1.0f / float(SAMPLE_COUNT);
const float PI = 3.14159265359f;
const float TWO_PI = PI * 2.0f;
const float HALF_PI = PI * 0.5f;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

vec3 getSampleDirection() {
    vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(o_prefilterMap));
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 direction;
    if (gl_GlobalInvocationID.z == 0)      direction = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) direction = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) direction = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) direction = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) direction = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) direction = vec3(-uv.x, uv.y,  -1.0);
    return normalize(direction);
}

float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i) {
    return vec2(float(i) * INVERSE_SAMPLE_COUNT, radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = TWO_PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 bitangent = cross(N, vec3(0.0f, 1.0f, 0.0f));
    bitangent = normalize(mix(cross(N, vec3(1.0f, 0.0f, 0.0f)), bitangent, step(0.00001f, dot(bitangent, bitangent))));
    vec3 tangent = normalize(cross(N, bitangent));

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float distributionGGX(float NdotH, float roughness) {
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (NdotH * NdotH) * (alphaSq - 1.0f) + 1.0f;
    return alphaSq / (PI * denom * denom);
}

void main() {
    vec2 inputSize = vec2(textureSize(u_cubeMap, 0));
    float saTexel = 4.0f * PI / (6.0f * inputSize.x * inputSize.y);

    vec3 N = getSampleDirection();
    vec3 R = N;
    vec3 V = R;

    float totalWeight = 0.0f;
    vec3 prefilteredColor = vec3(0.0f);

    for (uint i = 0u; i < SAMPLE_COUNT; i++) {
        vec2 Xi = hammersley(i);
        vec3 H = importanceSampleGGX(Xi, N, u_Roughness);
        vec3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        float NdotH = max(dot(N, H), 0.0f);
        float D = distributionGGX(NdotH, u_Roughness);
        float HdotV = max(dot(H, V), 0.0f);
        float pdf = D * NdotH / (4.0f * HdotV) + 0.0001f;

        float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf);
        float mipLevel = u_Roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

        prefilteredColor += textureLod(u_cubeMap, L, mipLevel).rgb * NdotL;
        totalWeight += NdotL;
    }
    prefilteredColor /= totalWeight;

    imageStore(o_prefilterMap, ivec3(gl_GlobalInvocationID), vec4(prefilteredColor, 1.0));
}
