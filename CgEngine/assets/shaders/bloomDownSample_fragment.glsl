#version 450 core

#include "common/CameraDataBuffer.glsl"

in VS_OUT {
    vec2 TexCoord;
} fs_in;

layout (binding = 0) uniform sampler2D u_BloomTexture;

out vec3 o_FragColor;

uniform bool u_UseThreshold;

vec3 prefilter(vec3 color) {
    // https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/ (3.2 Bloom Threshold)

    float brightness =  max(color.r, max(color.g, color.b));
    float contribution = max(0, brightness - u_CameraData.bloomThreshold);
    contribution /= max(brightness, 0.00001);
    return color * contribution;
}

void main() {
    vec2 texSize = 1.0f / vec2(textureSize(u_BloomTexture, 0));
    texSize *= 0.5f;
    float x = texSize.x;
    float y = texSize.y;

    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i

    vec3 a = texture(u_BloomTexture, vec2(fs_in.TexCoord.x - 2*x, fs_in.TexCoord.y + 2*y)).rgb;
    vec3 b = texture(u_BloomTexture, vec2(fs_in.TexCoord.x,       fs_in.TexCoord.y + 2*y)).rgb;
    vec3 c = texture(u_BloomTexture, vec2(fs_in.TexCoord.x + 2*x, fs_in.TexCoord.y + 2*y)).rgb;

    vec3 d = texture(u_BloomTexture, vec2(fs_in.TexCoord.x - 2*x, fs_in.TexCoord.y)).rgb;
    vec3 e = texture(u_BloomTexture, vec2(fs_in.TexCoord.x,       fs_in.TexCoord.y)).rgb;
    vec3 f = texture(u_BloomTexture, vec2(fs_in.TexCoord.x + 2*x, fs_in.TexCoord.y)).rgb;

    vec3 g = texture(u_BloomTexture, vec2(fs_in.TexCoord.x - 2*x, fs_in.TexCoord.y - 2*y)).rgb;
    vec3 h = texture(u_BloomTexture, vec2(fs_in.TexCoord.x,       fs_in.TexCoord.y - 2*y)).rgb;
    vec3 i = texture(u_BloomTexture, vec2(fs_in.TexCoord.x + 2*x, fs_in.TexCoord.y - 2*y)).rgb;

    vec3 j = texture(u_BloomTexture, vec2(fs_in.TexCoord.x - x, fs_in.TexCoord.y + y)).rgb;
    vec3 k = texture(u_BloomTexture, vec2(fs_in.TexCoord.x + x, fs_in.TexCoord.y + y)).rgb;
    vec3 l = texture(u_BloomTexture, vec2(fs_in.TexCoord.x - x, fs_in.TexCoord.y - y)).rgb;
    vec3 m = texture(u_BloomTexture, vec2(fs_in.TexCoord.x + x, fs_in.TexCoord.y - y)).rgb;

    // inner
    o_FragColor = (j+k+l+m) * 0.5f;

    // top-left
    o_FragColor += (a+b+d+e) * 0.125f;

    // top-right
    o_FragColor += (b+c+e+f) * 0.125f;

    // bottom-left
    o_FragColor += (d+e+g+h) * 0.125f;

    // bottom-right
    o_FragColor += (e+f+h+i) * 0.125f;

    o_FragColor *= 0.25f;

    if (u_UseThreshold) {
        o_FragColor = prefilter(o_FragColor);
    }

    o_FragColor = max(o_FragColor, 0.0001f);
}
