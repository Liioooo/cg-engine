#version 450 core

#include "Bloom.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout (binding = 0) uniform sampler2D u_BloomTexture;

layout(location = 0) out vec3 o_FragColor;

void main() {
    o_FragColor = upsampleBloom(u_BloomTexture, fs_in.TexCoord);
}
