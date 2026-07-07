#version 450 core

#include "LightDataBuffer.glsl"
#include "GBuffersFragment.glsl"

layout(location = 12) in VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec4 TexCoord;
    vec4 LodScales;
    vec3 ViewVector;
} fs_in;

layout (binding = 2, std140) uniform OceanData {
    vec3 foamColor;
    float roughness;
    vec3 sssColor;
    float roughnessScale;
    vec3 color;
    float maxGloss;
    float foamBias;
    float foamScale;
    float length0;
    float length1;
    float length2;
} u_OceanData;

uniform layout(binding=10) sampler2D u_displacementC0;
uniform layout(binding=11) sampler2D u_derivativesC0;
uniform layout(binding=12) sampler2D u_turbulenceC0;

uniform layout(binding=13) sampler2D u_displacementC1;
uniform layout(binding=14) sampler2D u_derivativesC1;
uniform layout(binding=15) sampler2D u_turbulenceC1;

uniform layout(binding=17) sampler2D u_displacementC2;
uniform layout(binding=18) sampler2D u_derivativesC2;
uniform layout(binding=19) sampler2D u_turbulenceC2;

void main() {
    vec4 derivatives = vec4(0.0);
    derivatives += texture(u_derivativesC0, fs_in.WorldPosition.xz / u_OceanData.length0);
    derivatives += texture(u_derivativesC1, fs_in.WorldPosition.xz / u_OceanData.length1) * fs_in.LodScales.y;
    derivatives += texture(u_derivativesC2, fs_in.WorldPosition.xz / u_OceanData.length2) * fs_in.LodScales.z;

    vec2 slope = vec2(derivatives.x / (1 + derivatives.z), derivatives.y / (1 + derivatives.w));
    vec3 N = normalize(vec3(-slope.x, 1, -slope.y));

    float jacobian = texture(u_turbulenceC0, fs_in.WorldPosition.xz / u_OceanData.length0).x
        + texture(u_turbulenceC1, fs_in.WorldPosition.xz / u_OceanData.length1).x
        + texture(u_turbulenceC2, fs_in.WorldPosition.xz / u_OceanData.length2).x;
    jacobian = min(1.0, max(0.0, (-jacobian + u_OceanData.foamBias) * u_OceanData.foamScale));

    vec3 _albedo = mix(vec3(0.0), u_OceanData.foamColor, jacobian);
    float distanceGloss = mix(1 - u_OceanData.roughness, u_OceanData.maxGloss, 1 / (1 + length(fs_in.ViewVector) * u_OceanData.roughnessScale));
    float _smoothness = mix(distanceGloss, 0.0, jacobian);

    vec3 _viewDir = normalize(fs_in.ViewVector);
    vec3 H = normalize(-N + u_LightData.dirLightDirection.xyz);
    float ViewDotH = pow(clamp(dot(_viewDir, -H), 0.0, 1.0), 5.0) * 30.0 * 0.133;
    vec3 color = mix(u_OceanData.color, clamp(u_OceanData.color + vec3(u_OceanData.sssColor * ViewDotH * fs_in.LodScales.w), 0.0, 1.0), fs_in.LodScales.z);

    float fresnel = dot(N, _viewDir);
    fresnel = clamp(1 - fresnel, 0.0, 1.0);
    fresnel = pow(fresnel, 5.0);

    // Unity uses perceptual smoothness, so we need to convert it into actual roughness
    // @see https://discussions.unity.com/t/roughness-to-smoothness-the-proper-way-to-convert-it/948632/2
    float _roughness = clamp(1 - _smoothness, 0.0, 1.0);
    _roughness = _roughness * _roughness;
    outputToGBuffers(
        _albedo,
        _roughness,
        mix(color * (1 - fresnel), vec3(0.0), jacobian),
        0.0,
        N,
        N
    );
}
