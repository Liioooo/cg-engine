#version 450 core

layout(early_fragment_tests) in;

uniform vec3 u_Color;

out vec4 o_FragColor;

void main() {
    o_FragColor = vec4(u_Color.rgb, 1.0f);
}
