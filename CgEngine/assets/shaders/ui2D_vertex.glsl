#version 450 core

#include "Macros.glsl"
#include "CameraDataBuffer.glsl"

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

PUSH_CONSTANT(UI2DPushConstants, 10) {
    mat4 transform;
} pc_ui2D;

layout(location = 10) out VS_OUT {
    vec2 TexCoord;
} vs_out;

void main() {
    gl_Position = u_CameraData.uiProjectionMatrix * pc_ui2D.transform * vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
    vs_out.TexCoord = a_TexCoord;
}
