#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/GBuffersVertex.glsl"

layout (quads, fractional_even_spacing, ccw) in;

uniform mat4 u_Transform;

uniform layout(binding=10) sampler2D u_displacementC0;
uniform layout(binding=11) sampler2D u_derivativesC0;
uniform layout(binding=12) sampler2D u_turbulenceC0;

uniform layout(binding=13) sampler2D u_displacementC1;
uniform layout(binding=14) sampler2D u_derivativesC1;
uniform layout(binding=15) sampler2D u_turbulenceC1;

uniform layout(binding=16) sampler2D u_displacementC2;
uniform layout(binding=17) sampler2D u_derivativesC2;
uniform layout(binding=18) sampler2D u_turbulenceC2;

in TS_OUT {
    vec4 aPos;
    vec4 aNormal;
    vec4 aTangent;
    vec4 aBitangent;
    vec4 aTexCoord;
} ts_in[];

out VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec4 TexCoord;
    vec4 LodScales;
    vec3 ViewVector;
} vs_out;

vec4 bilinearInterpolation(vec4 i00, vec4 i01, vec4 i10, vec4 i11) {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 i0 = (i01 - i00) * u + i00;
    vec4 i1 = (i11 - i10) * u + i10;
    return ((i1 - i0) * v + i0);
}

void main() {
    vec4 p = bilinearInterpolation(ts_in[0].aPos, ts_in[1].aPos, ts_in[2].aPos, ts_in[3].aPos);
    vec4 n = bilinearInterpolation(ts_in[0].aNormal, ts_in[1].aNormal, ts_in[2].aNormal, ts_in[3].aNormal);
    vec4 t = bilinearInterpolation(ts_in[0].aTexCoord, ts_in[1].aTexCoord, ts_in[2].aTexCoord, ts_in[3].aTexCoord);

    float length0 = 250;
    float length1 = 17;
    float length2 = 5;
    vec4 displacement0 = texture(u_displacementC0, (u_Transform * p).xz / length0);
    vec4 displacement1 = texture(u_displacementC1, (u_Transform * p).xz / length1);
    vec4 displacement2 = texture(u_displacementC2, (u_Transform * p).xz / length2);

    vs_out.ViewVector = vec3(u_CameraData.position - u_Transform * p);
    float viewDist = length(vs_out.ViewVector);
    float lod_c0 = min(7.13 * length0 / viewDist, 1);
    float lod_c1 = min(7.13 * length1 / viewDist, 1);
    float lod_c2 = min(7.13 * length2 / viewDist, 1);

    vec4 displacement = displacement0 * lod_c0;
    float largeWaveBias = displacement.y;
    displacement += displacement1 * lod_c1 + displacement2 * lod_c2;
    vec4 worldPosition = u_Transform * p + displacement;

    vs_out.Normal = mat3(transpose(inverse(u_Transform))) * n.xyz;
    vs_out.WorldPosition = worldPosition.xyz;
    vs_out.TexCoord = t;

    vs_out.LodScales = vec4(lod_c0, lod_c1, lod_c2, max(displacement.y - largeWaveBias * 0.8 - (-0.1), 0) / 4.8);

    gl_Position = u_CameraData.viewProjection * worldPosition;
    passGBufferData();
}

