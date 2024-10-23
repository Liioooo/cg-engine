#version 450 core

layout(vertices = 4) out;

in  mat4 vs_instanceTransform[];
out mat4 tcs_instanceTransform[];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_instanceTransform[gl_InvocationID] = vs_instanceTransform[gl_InvocationID];

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 3;
        gl_TessLevelOuter[1] = 3;
        gl_TessLevelOuter[2] = 3;
        gl_TessLevelOuter[3] = 3;

        gl_TessLevelInner[0] = 3;
        gl_TessLevelInner[1] = 3;
    }
}

