#version 450 core

layout(local_size_x = 1, local_size_y = 8, local_size_z = 1) in;

const float PI = 3.141592653589793;

layout(rgba32f, binding = 0) uniform image2D u_precomputeBuffer;

uniform int u_size;

vec2 ComplexExp(vec2 a) {
    return vec2(cos(a.y), sin(a.y)) * exp(a.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    uint b = u_size >> (id.x + 1);
    vec2 mult = 2 * PI * vec2(0, 1) / u_size;
    uint i = (2 * b * (id.y / b) + id.y % b) % u_size;
    vec2 twiddle = ComplexExp(-mult * ((id.y / b) * b));
    imageStore(u_precomputeBuffer, ivec2(id.xy), vec4(twiddle.xy, i, i + b));
    imageStore(u_precomputeBuffer, ivec2(id.x, id.y + u_size / 2), vec4(-twiddle.xy, i, i + b));
}