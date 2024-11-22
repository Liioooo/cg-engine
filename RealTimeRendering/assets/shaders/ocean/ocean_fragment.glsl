#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/DirShadowMappingFragment.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/IBLCalculationsFragment.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    vec4 TexCoord;
} fs_in;

uniform layout(location=0) sampler2D u_displacement;
uniform layout(location=1) sampler2D u_derivatives;
uniform layout(location=2) sampler2D u_turbulance;

out vec4 o_FragColor;

void main() {
    vec3 N = texture(u_derivatives, fs_in.TexCoord.xy).xyz;

    vec3 V = normalize(u_CameraData.position.xyz - fs_in.WorldPosition);
    float NdotV = max(dot(N, V), 0.0f);

    o_FragColor = vec4(0.6f, 0.6f, 0.6f, 1.0f) * (1.0f - calcDirShadow(fs_in.Normal, u_LightData.dirLightDirection.xyz, u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
    o_FragColor = texture(u_displacement, fs_in.TexCoord.xy);
    o_FragColor = vec4(calcIBL(vec3(1.333, 1.333, 1.333), vec3(0.0, 0.0, 1.0), 0.0f, 1.0, N, V, NdotV), 1.0);
}
