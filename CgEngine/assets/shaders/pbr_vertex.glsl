#version 450 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

out VS_OUT {
    vec2 TexCoord;
} vs_out;

void main() {
    gl_Position = vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
    vs_out.TexCoord = a_TexCoord;
}
