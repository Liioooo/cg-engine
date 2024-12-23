#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform restrict image2D u_h0Texture;

uniform int u_size;

void main() {
    uvec3 id = gl_GlobalInvocationID;
    vec2 h0K = imageLoad(u_h0Texture, ivec2(id.xy)).xy;
    vec2 h0MinusK = imageLoad(u_h0Texture, ivec2((u_size - id.x) % u_size, (u_size - id.y) % u_size)).xy;
    imageStore(u_h0Texture, ivec2(id.xy), vec4(h0K.xy, h0MinusK.x, -h0MinusK.y));
}