#version 450 core

#include "CameraDataBuffer.glsl"
#include "GBuffersVertex.glsl"
#include "TransformsOffsetPC.glsl"
#include "Macros.glsl"

layout(binding = 1, std430) buffer Transforms {
    mat4 transforms[];
} b_Transforms;

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

layout(location = 10) out VS_OUT {
    vec2 TexCoord;
    mat3 TBN;
    vec3 Normal;
} vs_out;

void main() {
    mat4 model = b_Transforms.transforms[pc_transformsOffset.transformsOffset + GET_INSTANCE_INDEX()];

    vec4 worldPosition = model * a_Pos;

    vs_out.TexCoord = a_TexCoord.xy;
    vs_out.TBN = mat3(model) * mat3(a_Tangent.xyz, a_Bitangent.xzy, a_Normal.xyz);
    vs_out.Normal = mat3(transpose(inverse(model))) * a_Normal.xyz;

    gl_Position = u_CameraData.viewProjection * worldPosition;

    passGBufferData();
}
