#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform restrict readonly image2D u_h0Texture;
layout(rgba32f, binding = 1) uniform restrict readonly image2D u_waveTexture;
layout(rg32f, binding = 2) uniform restrict writeonly image2D u_DxDz;
layout(rg32f, binding = 3) uniform restrict writeonly image2D u_DyDxz;
layout(rg32f, binding = 4) uniform restrict writeonly image2D u_DyxDyz;
layout(rg32f, binding = 5) uniform restrict writeonly image2D u_DxxDzz;

uniform float u_time;

vec2 complexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 wave = imageLoad(u_waveTexture, texelCoord);
    vec4 h0 = imageLoad(u_h0Texture, texelCoord);
    float phase = wave.w * u_time;
    vec2 exponent = vec2(cos(phase), sin(phase));

    // TODO: Check this
    vec2 h = 0.5 * complexMult(h0.xy, exponent) + 0.5 * complexMult(h0.zw, vec2(exponent.x, -exponent.y));
    vec2 ih = vec2(-h.y, h.x);
    vec2 dx = ih * wave.x * wave.y;
    vec2 dy = h;
    vec2 dz = ih * wave.z * wave.y;

    vec2 dx_dx = -h * wave.x * wave.x * wave.y;
    vec2 dy_dx = ih * wave.x;
    vec2 dz_dx = -h * wave.x * wave.z * wave.y;

    vec2 dy_dz = ih * wave.z;
    vec2 dz_dz = -h * wave.z * wave.z * wave.y;

    imageStore(u_DxDz, texelCoord, vec4(dx.x - dz.y, dx.y + dz.x, 0.0, 0.0));
    imageStore(u_DyDxz, texelCoord, vec4(dy.x - dz_dx.y, dy.y + dz_dx.x, 0.0, 0.0));
    imageStore(u_DyxDyz, texelCoord, vec4(dy_dx.x - dy_dz.y, dy_dx.y + dy_dz.x, 0.0, 0.0));
    imageStore(u_DxxDzz, texelCoord, vec4(dx_dx.x - dz_dz.y, dx_dx.y + dz_dz.x, 0.0, 0.0));
}
