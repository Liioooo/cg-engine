/*
From https://github.com/nvpro-samples/gl_ssao/blob/master/hbao_reinterleave.frag.glsl
*/

#version 450 core

layout(binding = 0) uniform sampler2DArray u_TexResultsArray;
layout(location = 0) out vec4 out_Color;

void main() {
    ivec2 FullResPos = ivec2(gl_FragCoord.xy);
    ivec2 Offset = FullResPos & 3;
    int SliceId = Offset.y * 4 + Offset.x;
    ivec2 QuarterResPos = FullResPos >> 2;

    out_Color = vec4(texelFetch(u_TexResultsArray, ivec3(QuarterResPos, SliceId), 0).xy, 0.0f, 0.0f);
}
