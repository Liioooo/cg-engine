#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rg32f, binding = 0) uniform image2D buffer0;

void main() {
    uvec3 id = gl_GlobalInvocationID;
    imageStore(buffer0, ivec2(id.xy), imageLoad(buffer0, ivec2(id.xy)) * (1.0 - 2.0 * ((id.x + id.y) % 2)));
}
