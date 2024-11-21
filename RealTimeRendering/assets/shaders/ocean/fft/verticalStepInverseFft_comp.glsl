#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const float PI = 3.141592653589793;

layout(rgba32f, binding = 0) uniform image2D u_precomputedData;
layout(rg32f, binding = 1) uniform image2D u_buffer0;
layout(rg32f, binding = 2) uniform image2D u_buffer1;

uniform bool u_pingPong;
uniform int u_step;

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    vec4 data = imageLoad(u_precomputedData, ivec2(u_step, id.y));
    uvec2 inputsIndices = uvec2(data.ba);
    if (u_pingPong) {
        vec2 value = imageLoad(u_buffer0, ivec2(id.x, inputsIndices.x)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(u_buffer0, ivec2(id.x, inputsIndices.y)).xy);
        imageStore(u_buffer1, ivec2(id.xy), vec4(value, 0, 1));
    } else {
        vec2 value = imageLoad(u_buffer1, ivec2(id.x, inputsIndices.x)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(u_buffer1, ivec2(id.x, inputsIndices.y)).xy);
        imageStore(u_buffer0, ivec2(id.xy), vec4(value, 0, 1));
    }
}