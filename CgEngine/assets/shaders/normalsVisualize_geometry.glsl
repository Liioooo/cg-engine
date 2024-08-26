#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 Normal;
} fs_in[];

void generateLine(int index) {
    gl_Position = u_CameraData.projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = u_CameraData.projection * (gl_in[index].gl_Position + vec4(fs_in[index].Normal, 0.0) * 0.2);
    EmitVertex();
    EndPrimitive();
}

void main() {
    generateLine(0);
    generateLine(1);
    generateLine(2);
}
