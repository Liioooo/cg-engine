#version 450 core

#include "common/Utilities.glsl"
#include "common/GBuffersFragment.glsl"

in VS_OUT {
    vec3 Normal0;
    vec3 Normal1;
    vec3 GrassParams; // x: heightPercent, y: xSide, z: highLODOut
    vec3 GrassColor;
} fs_in;

out vec4 o_FragColor;

void main() {
    vec3 normal = normalize(mix(fs_in.Normal0, fs_in.Normal1, fs_in.GrassParams.y));

    float heightPercent = fs_in.GrassParams.x;
    float highLODOut = fs_in.GrassParams.z;
    float xSide = fs_in.GrassParams.y;

    float grassMiddle = mix(smoothstep(abs(xSide - 0.5f), 0.0f, 0.1f), 1.0, 0.0f);
    float ao = easeIn(heightPercent, 2.0f);

    vec3 color = fs_in.GrassColor;
    color.rgb *= mix(0.85f, 1.0f, grassMiddle);
    color.rgb *= ao;

    float roughness = 0.8f;
    float metalic = 0.0f;
    vec3 emission = vec3(0.0f);

    outputToGBuffers(color, roughness, emission, metalic, normal, normal);
}
