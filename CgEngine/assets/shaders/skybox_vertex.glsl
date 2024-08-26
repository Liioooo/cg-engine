#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (location = 0) in vec3 a_Pos;

out VS_OUT {
    vec3 TexCoord;
} vs_out;

void main() {
    vs_out.TexCoord = a_Pos;
    vec4 pos = u_CameraData.projection * mat4(mat3(u_CameraData.view)) * vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
    gl_Position = pos.xyww;

}
