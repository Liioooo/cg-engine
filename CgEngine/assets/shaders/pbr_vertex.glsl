#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (binding = 2, std140) uniform DirShadowData {
    mat4 lightSpaceMat[4];
    vec4 cascadeSplits;
} u_DirShadowData;

layout(binding = 0, std430) buffer Transforms {
    mat4 transforms[];
} b_Transforms;

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

out VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    mat3 TBN;
    vec2 TexCoord;
} vs_out;

void main() {
    mat4 model = b_Transforms.transforms[gl_InstanceID];

    vec4 worldPosition = model * a_Pos;

    vs_out.TexCoord = a_TexCoord.xy;
    vs_out.WorldPosition = worldPosition.xyz;
    vs_out.Normal = mat3(transpose(inverse(model))) * a_Normal.xyz;
    vs_out.TBN = mat3(model) * mat3(a_Tangent.xyz, a_Bitangent.xzy, a_Normal.xyz);
    vs_out.DirShadowMapPosition[0] = u_DirShadowData.lightSpaceMat[0] * vec4(worldPosition.xyz, 1.0f);
    vs_out.DirShadowMapPosition[1] = u_DirShadowData.lightSpaceMat[1] * vec4(worldPosition.xyz, 1.0f);
    vs_out.DirShadowMapPosition[2] = u_DirShadowData.lightSpaceMat[2] * vec4(worldPosition.xyz, 1.0f);
    vs_out.DirShadowMapPosition[3] = u_DirShadowData.lightSpaceMat[3] * vec4(worldPosition.xyz, 1.0f);

    gl_Position = u_CameraData.viewProjection * worldPosition;
}
