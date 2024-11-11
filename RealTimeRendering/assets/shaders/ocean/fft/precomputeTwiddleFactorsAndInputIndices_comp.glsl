#version 450 core

layout(local_size_x = 1, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D precomputeBuffer;

uniform int size;

#define PI 3.141592653589793

vec2 ComplexExp(vec2 a) {
    return vec2(cos(a.y), sin(a.y)) * exp(a.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    uint b = size >> (id.x + 1);
    vec2 mult = 2 * PI * vec2(0, 1) / size;
    uint i = (2 * b * (id.y / b) + id.y % b) % size;
    vec2 twiddle = ComplexExp(-mult * ((id.y / b) * b));
    imageStore(precomputeBuffer, ivec2(id.xy), vec4(twiddle.xy, i, i + b));
    imageStore(precomputeBuffer, ivec2(id.x, id.y + size / 2), vec4(-twiddle.xy, i, i + b));
}