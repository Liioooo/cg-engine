/*
From https://github.com/nvpro-samples/gl_ssao/blob/master/hbao_blur.frag.glsl
*/

#version 450 core

#pragma optionNV(unroll all)

#include "Macros.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

const float KERNEL_RADIUS = 3;

PUSH_CONSTANT(HbaoBlurPC, 10) {
    float sharpness;
    vec2 invResolutionDirection;
} pc_hbaoBlur;

layout(binding = 0) uniform sampler2D u_InputTex;

layout(location = 0) out vec4 out_Color;

float BlurFunction(vec2 uv, float r, float center_c, float center_d, inout float w_total) {
    vec2 aoz = texture(u_InputTex, uv).xy;
    float c = aoz.x;
    float d = aoz.y;

    const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
    const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);

    float ddiff = (d - center_d) * pc_hbaoBlur.sharpness;
    float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
    w_total += w;

    return c*w;
}

void main() {
    vec2  aoz = texture(u_InputTex, fs_in.TexCoord).xy;
    float center_c = aoz.x;
    float center_d = aoz.y;

    float c_total = center_c;
    float w_total = 1.0;

    for (float r = 1; r <= KERNEL_RADIUS; ++r) {
        vec2 uv = fs_in.TexCoord + pc_hbaoBlur.invResolutionDirection * r;
        c_total += BlurFunction(uv, r, center_c, center_d, w_total);
    }

    for (float r = 1; r <= KERNEL_RADIUS; ++r) {
        vec2 uv = fs_in.TexCoord - pc_hbaoBlur.invResolutionDirection * r;
        c_total += BlurFunction(uv, r, center_c, center_d, w_total);
    }

    out_Color = vec4(c_total / w_total, center_d, 0, 1.0);
}
