#version 450 core

#include "CameraDataBuffer.glsl"
#include "Bloom.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout (binding = 0) uniform sampler2D u_FinalImage;
layout (binding = 1) uniform sampler2D u_BloomTexture;

layout(location = 0) out vec4 o_FragColor;

vec3 toneMap(vec3 color, float exposure) {
    return vec3(1.0) - exp(-color * exposure);
}

void main() {
    vec4 hdrColor = texture(u_FinalImage, fs_in.TexCoord);

    vec3 bloom = upsampleBloom(u_BloomTexture, fs_in.TexCoord) * u_CameraData.bloomIntensity;

    hdrColor += vec4(bloom, 1.0f);
    hdrColor *= u_CameraData.exposure;

    vec3 fragColor = toneMap(hdrColor.rgb, 1);
    o_FragColor = vec4(fragColor, 1.0f);
}
