#version 450 core

in vec2 tessUV;

out vec4 o_FragColor;

void main() {
    o_FragColor = vec4(tessUV, 0.0f, 1.0f);
}
