#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/IBLCalculationsFragment.glsl"
#include "common/GBuffersFragment.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec3 Normal;
    vec4 TexCoord;
    vec4 LodScales;
    vec3 ViewVector;
} fs_in;

uniform float u_length0;
uniform float u_length1;
uniform float u_length2;
uniform vec3 u_foamColor;
uniform vec3 u_sssColor;
uniform vec3 u_color;
uniform float u_roughness;
uniform float u_roughnessScale;
uniform float u_maxGloss;
uniform float u_foamBias;
uniform float u_foamScale;

uniform layout(binding=10) sampler2D u_displacementC0;
uniform layout(binding=11) sampler2D u_derivativesC0;
uniform layout(binding=12) sampler2D u_turbulenceC0;

uniform layout(binding=13) sampler2D u_displacementC1;
uniform layout(binding=14) sampler2D u_derivativesC1;
uniform layout(binding=15) sampler2D u_turbulenceC1;

uniform layout(binding=16) sampler2D u_displacementC2;
uniform layout(binding=17) sampler2D u_derivativesC2;
uniform layout(binding=18) sampler2D u_turbulenceC2;

out vec4 o_FragColor;

void main() {
    vec4 derivatives = vec4(0.0);
    derivatives += texture(u_derivativesC0, fs_in.WorldPosition.xz / u_length0);
    derivatives += texture(u_derivativesC1, fs_in.WorldPosition.xz / u_length1) * fs_in.LodScales.y;
    derivatives += texture(u_derivativesC2, fs_in.WorldPosition.xz / u_length2) * fs_in.LodScales.z;

    vec2 slope = vec2(derivatives.x / (1 + derivatives.z), derivatives.y / (1 + derivatives.w));
    vec3 N = normalize(vec3(-slope.x, 1, -slope.y));

    float jacobian = texture(u_turbulenceC0, fs_in.WorldPosition.xz / u_length0).x
        + texture(u_turbulenceC1, fs_in.WorldPosition.xz / u_length1).x
        + texture(u_turbulenceC2, fs_in.WorldPosition.xz / u_length2).x;
    jacobian = min(1.0, max(0.0, (-jacobian + u_foamBias) * u_foamScale));

    vec3 _albedo = mix(vec3(0.0), u_foamColor, jacobian);
    float distanceGloss = mix(1 - u_roughness, u_maxGloss, 1 / (1 + length(fs_in.ViewVector) * u_roughnessScale));
    float _smoothness = mix(distanceGloss, 0.0, jacobian);

    vec3 _viewDir = normalize(fs_in.ViewVector);
    vec3 H = normalize(-N + u_LightData.dirLightDirection.xyz);
    float ViewDotH = pow(clamp(dot(_viewDir, -H), 0.0, 1.0), 5.0) * 30.0 * 0.133;
    vec3 color = mix(u_color, clamp(u_color + vec3(u_sssColor * ViewDotH * fs_in.LodScales.w), 0.0, 1.0), fs_in.LodScales.z);

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
