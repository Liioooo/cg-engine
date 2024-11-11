#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/DirShadowMappingFragment.glsl"
#include "common/LightDataBuffer.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    vec4 TexCoord;
} fs_in;
uniform layout(location=0) sampler2D tex;

out vec4 o_FragColor;

void main() {
    o_FragColor = vec4(0.6f, 0.6f, 0.6f, 1.0f) * (1.0f - calcDirShadow(fs_in.Normal, u_LightData.dirLightDirection.xyz, u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
    o_FragColor = texture(tex, fs_in.TexCoord.xy);
}
