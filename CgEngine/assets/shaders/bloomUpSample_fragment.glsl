#version 450 core

#include "common/Bloom.glsl"

in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout (binding = 0) uniform sampler2D u_BloomTexture;

out vec3 o_FragColor;

void main() {
    o_FragColor = upsampleBloom(u_BloomTexture, fs_in.TexCoord);
}
