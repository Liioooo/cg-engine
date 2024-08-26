#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (location = 0) in vec4 a_PosUV;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in float a_FontAtlasIndex;

out VS_OUT {
    vec2 TexCoord;
    vec4 Color;
} vs_out;

out flat float FontAtlasIndex;

void main() {
    gl_Position = u_CameraData.uiProjectionMatrix * vec4(a_PosUV.x, a_PosUV.y, 0.0f, 1.0);
    vs_out.TexCoord = a_PosUV.zw;
    vs_out.Color = a_Color;
    FontAtlasIndex = a_FontAtlasIndex;
}
