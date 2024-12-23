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
    float length0 = 250;
    float length1 = 17;
    float length2 = 5;
    vec4 derivatives = vec4(0.0);
    derivatives += texture(u_derivativesC0, fs_in.TexCoord.xy);
    derivatives += texture(u_derivativesC1, fs_in.TexCoord.xy * (length0 / length1)) * fs_in.LodScales.y;
    derivatives += texture(u_derivativesC2, fs_in.TexCoord.xy * (length0 / length2)) * fs_in.LodScales.z;

    vec2 slope = vec2(derivatives.x / (1 + derivatives.z), derivatives.y / (1 + derivatives.w));
    vec3 N = normalize(vec3(-slope.x, 1, -slope.y));

//    #if defined(CLOSE)
    float jacobian = texture(u_turbulenceC0, fs_in.TexCoord.xy).x
        + texture(u_turbulenceC1, fs_in.TexCoord.xy * (length0 / length1)).x
        + texture(u_turbulenceC2, fs_in.TexCoord.xy * (length0 / length2)).x;
    jacobian = min(1.0, max(0.0, (-jacobian + 2.72) * 0.3));
//    #elif defined(MID)
//            float jacobian = tex2D(_Turbulence_c0, IN.worldUV / LengthScale0).x
//    + tex2D(_Turbulence_c1, IN.worldUV / LengthScale1).x;
//    jacobian = min(1, max(0, (-jacobian + _FoamBiasLOD1) * _FoamScale));
//    #else
//    float jacobian = texture(u_turbulenceC0, fs_in.TexCoord.xy).x;
//    jacobian = min(1, max(0, (-jacobian + 0.84) * 1.0));
//    #endif

//    vec2 screenUV = IN.screenPos.xy / IN.screenPos.w;
//    float backgroundDepth =
//    LinearEyeDepth(SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, screenUV));
//    float surfaceDepth = UNITY_Z_0_FAR_FROM_CLIPSPACE(IN.screenPos.z);
//    float depthDifference = max(0, backgroundDepth - surfaceDepth - 0.1);
//    float foam = tex2D(_FoamTexture, IN.worldUV * 0.5 + _Time.r).r;
//    jacobian += _ContactFoam * saturate(max(0, foam - depthDifference) * 5) * 0.9;

    vec4 foamColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 _albedo = mix(vec4(0.0), foamColor, jacobian);
    float roughness = 0.311;
    float roughnessScale = 0.0044;
    float maxGloss = 0.91;
    float distanceGloss = mix(1 - roughness, maxGloss, 1 / (1 + length(fs_in.ViewVector) * roughnessScale));
    float _smoothness = mix(distanceGloss, 0.0, jacobian);

    vec3 _viewDir = normalize(fs_in.ViewVector);
    vec3 H = normalize(-N + u_LightData.dirLightDirection.xyz);
    float ViewDotH = pow(clamp(dot(_viewDir, -H), 0.0, 1.0), 5.0) * 30.0 * 0.133;
    vec4 _sssColor = vec4(0.1541919, 0.8857628, 0.990566, 1.0);
    vec4 _color = vec4(0.03457636, 0.12297464, 0.1981132, 1.0);
    vec4 color = mix(_color, clamp(_color + vec4(_sssColor.rgb * ViewDotH * fs_in.LodScales.w, 0.0), 0.0, 1.0), fs_in.LodScales.z);

    float fresnel = dot(N, _viewDir);
    fresnel = clamp(1 - fresnel, 0.0, 1.0);
    fresnel = pow(fresnel, 5.0);

    // Unity uses perceptual smoothness, so we need to convert it into actual roughness
    // @see https://discussions.unity.com/t/roughness-to-smoothness-the-proper-way-to-convert-it/948632/2
    float _roughness = clamp(1 - _smoothness, 0.0, 1.0);
    _roughness = _roughness * _roughness;
    outputToGBuffers(
        _albedo.rgb,
        _roughness,
        mix(color * (1 - fresnel), vec4(0.0), jacobian).rgb,
        0.0,
        N,
        N
    );
}
