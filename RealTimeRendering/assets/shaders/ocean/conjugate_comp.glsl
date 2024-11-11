#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D h0_texture;

int N = 256;

void main() {
    uvec3 id = gl_GlobalInvocationID;
    vec2 h0K = imageLoad(h0_texture, ivec2(id.xy)).xy;
    vec2 h0MinusK = imageLoad(h0_texture, ivec2((N - id.x) % N, (N - id.y) % N)).xy;
    imageStore(h0_texture, ivec2(id.xy), vec4(h0K.xy, h0MinusK.x, -h0MinusK.y));
}