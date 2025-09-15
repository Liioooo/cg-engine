#version 450 core

layout(early_fragment_tests) in;

layout(location = 0) out vec4 o_FragColor;

layout(location = 10) in VS_OUT {
    vec3 Color;
} fs_in;

void main() {
    o_FragColor = vec4(fs_in.Color, 1.0f);
}
