#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform restrict image2D u_h0Texture;

void main() {
    uvec3 id = gl_GlobalInvocationID;
    int size = 256;
    vec2 h0K = imageLoad(u_h0Texture, ivec2(id.xy)).xy;
    vec2 h0MinusK = imageLoad(u_h0Texture, ivec2((size - id.x) % size, (size - id.y) % size)).xy;
    imageStore(u_h0Texture, ivec2(id.xy), vec4(h0K.xy, h0MinusK.x, -h0MinusK.y));
}
