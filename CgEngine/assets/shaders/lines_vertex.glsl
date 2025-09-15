#version 450 core

#include "CameraDataBuffer.glsl"

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Color;

layout(location = 10) out VS_OUT {
    vec3 Color;
} vs_out;

void main() {
    gl_Position = u_CameraData.viewProjection * vec4(a_Pos, 1.0f);
    vs_out.Color = a_Color;
}
