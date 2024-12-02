#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/DirLightCalculationsFragment.glsl"
#include "common/IBLCalculationsFragment.glsl"
#include "common/Utilities.glsl"

in VS_OUT {
    vec3 WorldPosition;
    vec3 Normal0;
    vec3 Normal1;
    vec3 GrassParams; // x: heightPercent, y: xSide, z: highLODOut
    vec3 GrassColor;
} fs_in;

out vec4 o_FragColor;

void main() {
    vec3 normal = normalize(mix(fs_in.Normal0, fs_in.Normal1, fs_in.GrassParams.y));

    vec3 V = normalize(u_CameraData.position.xyz - fs_in.WorldPosition);
    float NdotV = max(dot(normal, V), 0.0f);

    float heightPercent = fs_in.GrassParams.x;
    float highLODOut = fs_in.GrassParams.z;
    float xSide = fs_in.GrassParams.y;

    float grassMiddle = mix(smoothstep(abs(xSide - 0.5f), 0.0f, 0.1f), 1.0, 0.0f);
    float ao = easeIn(heightPercent, 2.0f);

    vec3 color = fs_in.GrassColor;
    color.rgb *= mix(0.85f, 1.0f, grassMiddle);
    color.rgb *= ao;

    vec3 F0 = calcF0(color, 0.0f);

    vec3 light = calcDirLight(F0, color, 0.0f, 0.8f, normal, V, NdotV);
    light += calcIBL(F0, color, 0.0f, 0.8f, normal, V, NdotV) * u_EnvironmentIntensity;

    o_FragColor = vec4(light, 1.0f);
}
