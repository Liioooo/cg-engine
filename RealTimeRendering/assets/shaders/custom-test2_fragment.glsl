#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/DirShadowMappingFragment.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
} fs_in;

out vec4 o_FragColor;

void main() {
    o_FragColor = vec4(0.0f, 1.0f, 0.f, 1.0f) * (1.0f - calcDirShadow(fs_in.Normal, normalize(vec3(1.0f, 0.0f, 0.0f)), u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
}
