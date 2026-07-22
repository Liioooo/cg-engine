#version 450 core

#include "Macros.glsl"

layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;

UNIFORM_LAYOUT_STD140(2, 0) uniform DirShadowData {
    mat4 lightSpaceMat[4];
    vec4 cascadeSplits;
} u_DirShadowData;

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = u_DirShadowData.lightSpaceMat[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
