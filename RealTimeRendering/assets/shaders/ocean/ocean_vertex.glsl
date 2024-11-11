#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/DirShadowMappingVertex.glsl"


layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

uniform mat4 u_Transform;

out VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    vec4 TexCoord;
} vs_out;

out vec2 frag_texCoord;

void main() {
    vec4 worldPosition = u_Transform * a_Pos + vec4(gl_InstanceID * 2.0f, 0.0f, 0.0f, 0.f);

    vs_out.DirShadowMapPosition = calcDirShadowMapPostion(worldPosition.xyz);
    vs_out.Normal = mat3(transpose(inverse(u_Transform))) * a_Normal.xyz;
    vs_out.WorldPosition = worldPosition.xyz;
    vs_out.TexCoord = a_TexCoord;

    gl_Position = u_CameraData.viewProjection * worldPosition;
}

