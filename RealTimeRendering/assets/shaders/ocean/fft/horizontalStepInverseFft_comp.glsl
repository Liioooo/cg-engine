#version 450 core

#include "Macros.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const float PI = 3.141592653589793;

UNIFORM_LAYOUT_FORMAT(0, 0, rgba32f) uniform restrict readonly image2D u_precomputedData;
UNIFORM_LAYOUT_FORMAT(1, 0, rg32f) uniform restrict image2D u_buffer0;
UNIFORM_LAYOUT_FORMAT(2, 0, rg32f)  uniform restrict image2D u_buffer1;

PUSH_CONSTANT(PCFFT) {
    int step;
    bool pingPong;
} pc_fft;

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    vec4 data = imageLoad(u_precomputedData, ivec2(pc_fft.step, id.x));
    uvec2 inputsIndices = uvec2(data.ba);
    if (pc_fft.pingPong) {
        vec2 value = imageLoad(u_buffer0, ivec2(inputsIndices.x, id.y)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(u_buffer0, ivec2(inputsIndices.y, id.y)).xy);
        imageStore(u_buffer1, ivec2(id.xy), vec4(value, 0.0, 1.0));
    } else {
        vec2 value = imageLoad(u_buffer1, ivec2(inputsIndices.x, id.y)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(u_buffer1, ivec2(inputsIndices.y, id.y)).xy);
        imageStore(u_buffer0, ivec2(id.xy), vec4(value, 0.0, 1.0));
    }
}
