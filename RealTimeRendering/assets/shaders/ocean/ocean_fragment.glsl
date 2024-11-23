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

uniform layout(binding=0) sampler2D u_displacement;
uniform layout(binding=1) sampler2D u_derivatives;
uniform layout(binding=2) sampler2D u_turbulance;

out vec4 o_FragColor;

void main() {
    vec4 derivatives = texture(u_derivatives, fs_in.TexCoord.xy);
    vec2 slope = vec2(derivatives.x / (1 + derivatives.z), derivatives.y / (1 + derivatives.w));
    vec3 N = normalize(vec3(-slope.x, 1, -slope.y));

    vec3 V = normalize(u_CameraData.position.xyz - fs_in.WorldPosition);
    float NdotV = max(dot(N, V), 0.0f);

    o_FragColor = vec4(0.6f, 0.6f, 0.6f, 1.0f) * (1.0f - calcDirShadow(fs_in.Normal, u_LightData.dirLightDirection.xyz, u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
    o_FragColor = texture(u_displacement, fs_in.TexCoord.xy);
    o_FragColor = vec4(calcIBL(vec3(1.333, 1.333, 1.333), vec3(0.0, 0.0, 1.0), 0.0f, 1.0, N, V, NdotV), 1.0);
    o_FragColor = vec4(N, 1.0);
}
