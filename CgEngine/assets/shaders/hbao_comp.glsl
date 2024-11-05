/*
From: https://github.com/nvpro-samples/gl_ssao/blob/master/hbao.frag.glsl and TheCherno/Hazel
*/

#version 450 core

#include "common/HBAODataBuffer.glsl"
#include "common/ScreenDataBuffer.glsl"
#include "common/PI.glsl"

layout(binding = 0) uniform sampler2DArray u_LinearDepth;
layout(binding = 1) uniform sampler2D u_ViewNormals;
layout(binding = 2, rg16f) restrict writeonly uniform image2DArray o_Output;

// tweakables
const float NUM_STEPS = 4;
const float NUM_DIRECTIONS = 8;

vec3 getQuarterCoord(vec2 UV) {
    return vec3(UV, float(gl_GlobalInvocationID.z));
}

vec3 uvToView(vec2 uv, float eye_z) {
    return vec3((uv * u_HBAO.perspectiveInfo.xy + u_HBAO.perspectiveInfo.zw) * (u_HBAO.isOrtho ? 1.0f : eye_z), eye_z);
}

vec3 FetchQuarterResViewPos(vec2 UV) {
    float ViewDepth = textureLod(u_LinearDepth, getQuarterCoord(UV),0).x;
    return uvToView(UV, ViewDepth);
}

float Falloff(float DistanceSquare) {
    // 1 scalar mad instruction
    return DistanceSquare * u_HBAO.negInvR2 + 1.0f;
}

// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
float ComputeAO(vec3 P, vec3 N, vec3 S) {
    vec3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * 1.0f / sqrt(VdotV);
    return clamp(NdotV - u_HBAO.nDotVBias, 0.0f, 1.0f) * clamp(Falloff(VdotV), 0.0f, 1.0f);
}

vec2 RotateDirection(vec2 Dir, vec2 CosSin) {
    return vec2(Dir.x * CosSin.x - Dir.y * CosSin.y, Dir.x * CosSin.y + Dir.y * CosSin.x);
}

vec4 GetJitter() {
    // Get the current jitter vector from the per-pass constant buffer
    return u_HBAO.jitters[gl_GlobalInvocationID.z];
}

float ComputeCoarseAO(vec2 fullResUV, float radiusPixels, vec4 rand, vec3 viewPosition, vec3 viewNormal) {
    radiusPixels /= 4.0;

    // Divide by NUM_STEPS + 1 so that the furthest samples are not fully attenuated
    float stepSizePixels = radiusPixels / (NUM_STEPS + 1);

    const float Alpha = 2.0 * PI / NUM_DIRECTIONS;
    float AO = 0;
    for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex) {
        float Angle = Alpha * DirectionIndex;

        // Compute normalized 2D direction
        vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), rand.xy);

        // Jitter starting sample within the first step
        float RayPixels = (rand.z * stepSizePixels + 1.0);

        for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex) {
            vec2 snappedUV = round(RayPixels * Direction) * u_HBAO.invQuarterResolution + fullResUV;
            vec3 S = FetchQuarterResViewPos(snappedUV);
            RayPixels += stepSizePixels;

            AO += ComputeAO(viewPosition, viewNormal, S);
        }
    }
    AO *= u_HBAO.aoMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
    return clamp(1.0f - AO * 2.0f, 0.0f, 1.0f);
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

void main() {
    vec2 float2Offset = u_HBAO.float2Offsets[gl_GlobalInvocationID.z].xy;
    vec2 base = gl_GlobalInvocationID.xy * 4.0f + float2Offset;
    vec2 uv = base * (u_HBAO.invQuarterResolution / 4.0f);

    vec3 viewPosition = FetchQuarterResViewPos(uv);
    vec3 normal = texelFetch(u_ViewNormals, ivec2(base), 0).xyz;
    vec3 viewNormal = normalize(normal);
    viewNormal.z = -viewNormal.z;

    // Compute projection of disk of radius control.R into screen space
    float RadiusPixels = u_HBAO.radiusToScreen / (u_HBAO.isOrtho ? 1.0f : viewPosition.z);

    // Get jitter vector for the current full-res pixel
    vec4 Rand = GetJitter();

    float AO = ComputeCoarseAO(uv, RadiusPixels, Rand, viewPosition, viewNormal);

    imageStore(o_Output, ivec3(gl_GlobalInvocationID), vec4(pow(AO, u_HBAO.powExponent), viewPosition.z, 0.0f, 0.0f));
}
