#version 450 core

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout (binding = 0) uniform sampler2D u_RenderedCanvas;

layout(location = 0) out vec4 o_FragColor;

void main() {
    o_FragColor = texture(u_RenderedCanvas, fs_in.TexCoord);
}
