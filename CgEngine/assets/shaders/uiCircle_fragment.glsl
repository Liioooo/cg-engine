#version 450 core

#include "Macros.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
    vec4 LineColor;
    vec4 FillColor;
    float LineWidth;
    float Diameter;
} fs_in;

layout(location = 20) in flat float TextureIndex;

layout(location = 0) out vec4 o_FragColor;

UNIFORM_LAYOUT(0, 0) uniform sampler2D u_Textures[16];

vec4 toLinearRGB(vec4 color) {
    return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
    vec2 localPos = (fs_in.TexCoord - 0.5f) * 2.0f;
    float dist = length(localPos);
    float aa = fwidth(dist);

    float circleMask = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, dist);

    if (circleMask <= 0.0) {
        discard;
    }

    float innerRadius = (fs_in.Diameter - fs_in.LineWidth) / fs_in.Diameter;
    float onLine = step(0.01f, fs_in.LineWidth) * smoothstep(innerRadius - aa, innerRadius + aa, dist);

    vec4 fillColor = TextureIndex < 0.0f ? toLinearRGB(fs_in.FillColor) : texture(u_Textures[int(TextureIndex)], fs_in.TexCoord);
    vec4 finalColor = mix(fillColor, toLinearRGB(fs_in.LineColor), onLine);
    finalColor.a *= circleMask;

    o_FragColor = finalColor;
}
