#version 450 core

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
    vec4 Color;
} fs_in;

layout(location = 20) in flat float FontAtlasIndex;

layout(location = 0) out vec4 o_FragColor;

layout(binding = 0) uniform sampler2D u_Textures[4];

vec4 toLinearRGB(vec4 color) {
    return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
    o_FragColor = toLinearRGB(fs_in.Color) * vec4(1.0f, 1.0f, 1.0f, texture(u_Textures[int(FontAtlasIndex)], fs_in.TexCoord).r);
}
