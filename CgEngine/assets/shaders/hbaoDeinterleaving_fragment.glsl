/*
From https://github.com/nvpro-samples/gl_ssao/blob/master/hbao_deinterleave.frag.glsl
*/

#version 450 core

#include "CameraDataBuffer.glsl"
#include "ScreenDataBuffer.glsl"
#include "Macros.glsl"

PUSH_CONSTANT(HbaoUvOffset) {
    int uvOffset;
} pc_uvOffset;


layout(binding = 0) uniform sampler2D u_Depth;

layout(location = 0) out float out_Color[8];

const vec2 UV_OFFSETS[2] = { vec2(0.5f, 0.5f), vec2(0.5f, 2.5f) };

// From: https://github.com/nvpro-samples/gl_ssao/blob/master/depthlinearize.frag.glsl
float linearizeDepth(const float screenDepth) {
    if (u_CameraData.clipInfo[3] != 0) {
        return (u_CameraData.clipInfo[0] / (u_CameraData.clipInfo[1] * screenDepth + u_CameraData.clipInfo[2]));
    } else {
        return (u_CameraData.clipInfo[1] + u_CameraData.clipInfo[2] - screenDepth * u_CameraData.clipInfo[1]);
    }
    /*
    if (in_perspective == 1.0) { // perspective
        ze = (zNear * zFar) / (zFar - screenDepth * (zFar - zNear));
    }
    else { // orthographic proj
        ze  = zNear + screenDepth  * (zFar - zNear);
    }
    */
}

vec4 linearizeDepth(vec4 deviceZs) {
    return vec4(linearizeDepth(deviceZs.x), linearizeDepth(deviceZs.y), linearizeDepth(deviceZs.z), linearizeDepth(deviceZs.w));
}

void main() {
    vec2 uv = floor(gl_FragCoord.xy) * 4.0f + UV_OFFSETS[pc_uvOffset.uvOffset] + 0.5f;
    uv *= u_ScreenData.invFullResolution;

    vec4 S0 = linearizeDepth(textureGather(u_Depth, uv, 0));
    vec4 S1 = linearizeDepth(textureGatherOffset(u_Depth, uv, ivec2(2, 0), 0));

    out_Color[0] = S0.w;
    out_Color[1] = S0.z;
    out_Color[2] = S1.w;
    out_Color[3] = S1.z;
    out_Color[4] = S0.x;
    out_Color[5] = S0.y;
    out_Color[6] = S1.x;
    out_Color[7] = S1.y;
}
