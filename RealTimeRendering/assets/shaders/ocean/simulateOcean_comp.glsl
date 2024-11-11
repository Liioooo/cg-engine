#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D h0_texture;
layout(rgba32f, binding = 1) uniform image2D wave_texture;
layout(rg32f, binding = 2) uniform image2D h_texture;
//layout(rgba32f, binding = 3) uniform image2D dx_texture;
//layout(rgba32f, binding = 4) uniform image2D dz_texture;

uniform float time = 0;

vec2 complexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 wave = imageLoad(wave_texture, texelCoord);
    vec4 h0 = imageLoad(h0_texture, texelCoord);
    float phase = wave.w * time;
    vec2 exponent = vec2(cos(phase), sin(phase));

    vec2 h = complexMult(h0.xy, exponent) + complexMult(h0.zw, vec2(exponent.x, -exponent.y));
    vec2 ih = vec2(-h.y, h.x);
    vec2 dx = ih * wave.x * wave.y;
    vec2 dz = ih * wave.z * wave.y;

//    imageStore(h_texture, texelCoord, vec4(dx.x - dz.y, dx.y + dz.x, 0, 1));

    imageStore(h_texture, texelCoord, vec4(h.xy, 0, 1));
//    imageStore(dx_texture, texelCoord, vec4(dx.xy, 0, 1));
//    imageStore(dz_texture, texelCoord, vec4(dz.xy, 0, 1));
}
