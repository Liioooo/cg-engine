#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (location = 0) in vec4 a_PosUV;
layout (location = 1) in vec4 a_LineColor;
layout (location = 2) in vec4 a_FillColor;
layout (location = 3) in vec2 a_Size;
layout (location = 4) in float a_LineWidth;
layout (location = 5) in float a_TextureIndex;

out VS_OUT {
    vec2 TexCoord;
    vec4 LineColor;
    vec4 FillColor;
    vec2 Size;
    float LineWidth;
} vs_out;

out flat float TextureIndex;

void main() {
    gl_Position = u_CameraData.uiProjectionMatrix * vec4(a_PosUV.x, a_PosUV.y, 0.0f, 1.0);
    vs_out.TexCoord = a_PosUV.zw;
    vs_out.LineColor = a_LineColor;
    vs_out.FillColor = a_FillColor;
    vs_out.Size = a_Size;
    vs_out.LineWidth = a_LineWidth;
    TextureIndex = a_TextureIndex;
}
