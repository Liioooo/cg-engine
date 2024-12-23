#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/GBuffersVertex.glsl"

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

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

out VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec4 TexCoord;
    vec4 LodScales;
    vec3 ViewVector;
} vs_out;

out vec2 frag_texCoord;

void main() {
    float length0 = 250;
    float length1 = 17;
    float length2 = 5;
    vec4 displacement0 = texture(u_displacementC0, a_TexCoord.xy);
    vec4 displacement1 = texture(u_displacementC1, a_TexCoord.xy * (length0 / length1));
    vec4 displacement2 = texture(u_displacementC2, a_TexCoord.xy * (length0 / length2));

    vs_out.ViewVector = vec3(u_CameraData.position - u_Transform * a_Pos);
    float viewDist = length(vs_out.ViewVector);
    float lod_c0 = min(7.13 * length0 / viewDist, 1);
    float lod_c1 = min(7.13 * length1 / viewDist, 1);
    float lod_c2 = min(7.13 * length2 / viewDist, 1);

    vec4 displacement = displacement0 * lod_c0;
    float largeWaveBias = displacement.y;
    displacement += displacement1 * lod_c1 + displacement2 * lod_c2;
    vec4 worldPosition = u_Transform * a_Pos + displacement;

    vs_out.Normal = mat3(transpose(inverse(u_Transform))) * a_Normal.xyz;
    vs_out.WorldPosition = worldPosition.xyz;
    vs_out.TexCoord = a_TexCoord;

    vs_out.LodScales = vec4(lod_c0, lod_c1, lod_c2, max(displacement.y - largeWaveBias * 0.8 - (-0.1), 0) / 4.8);

    gl_Position = u_CameraData.viewProjection * worldPosition;
    passGBufferData();
}

