#version 450 core

#include "Macros.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

UNIFORM_LAYOUT(1, 1) uniform sampler2D u_RenderedCanvas;

layout(location = 0) out vec4 o_FragColor;

void main() {
    o_FragColor = texture(u_RenderedCanvas, fs_in.TexCoord);
}
