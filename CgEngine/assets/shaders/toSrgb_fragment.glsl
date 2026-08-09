#version 450 core

#include "Macros.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

UNIFORM_LAYOUT(0, 0) uniform sampler2D u_FinalImage;

layout(location = 0) out vec4 o_FragColor;

vec3 toSRGB(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    vec4 color = texture(u_FinalImage, fs_in.TexCoord);
    color.rgb = toSRGB(color.rgb);
    o_FragColor = color;
}
