#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/DirShadowMappingVertex.glsl"

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

layout(binding = 5, std430) buffer InstanceBuffer {
    mat4 transforms[];
} b_InstanceBuffer;

uniform mat4 u_Transform;

out VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
} vs_out;

void main() {
    mat4 transfrom = u_Transform * b_InstanceBuffer.transforms[gl_InstanceID];

    vec4 worldPosition = transfrom * a_Pos;

    vs_out.DirShadowMapPosition = calcDirShadowMapPostion(worldPosition.xyz);
    vs_out.Normal = mat3(transpose(inverse(transfrom))) * a_Normal.xyz;
    vs_out.WorldPosition = worldPosition.xyz;

    gl_Position = u_CameraData.viewProjection * worldPosition;
}

