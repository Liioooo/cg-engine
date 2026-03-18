#version 450 core

#include "Macros.glsl"

layout(local_size_x = 1, local_size_y = 8, local_size_z = 1) in;

const float PI = 3.141592653589793;

layout(rgba32f, binding = 0) uniform restrict writeonly image2D u_precomputeBuffer;

PUSH_CONSTANT(PCPrecompute) {
    int size;
} pc_precompute;

vec2 ComplexExp(vec2 a) {
    return vec2(cos(a.y), sin(a.y)) * exp(a.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    uint b = pc_precompute.size >> (id.x + 1);
    vec2 mult = 2 * PI * vec2(0, 1) / pc_precompute.size;
    uint i = (2 * b * (id.y / b) + id.y % b) % pc_precompute.size;
    vec2 twiddle = ComplexExp(-mult * ((id.y / b) * b));
    imageStore(u_precomputeBuffer, ivec2(id.xy), vec4(twiddle.xy, i, i + b));
    imageStore(u_precomputeBuffer, ivec2(id.x, id.y + pc_precompute.size / 2), vec4(-twiddle.xy, i, i + b));
}
