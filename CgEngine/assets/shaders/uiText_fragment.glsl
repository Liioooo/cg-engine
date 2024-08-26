#version 450 core

in VS_OUT {
    vec2 TexCoord;
    vec4 Color;
} fs_in;

in flat float FontAtlasIndex;

out vec4 o_FragColor;

layout(binding = 0) uniform sampler2D u_Textures[4];

void main() {
    o_FragColor = fs_in.Color * vec4(1.0f, 1.0f, 1.0f, texture(u_Textures[int(FontAtlasIndex)], fs_in.TexCoord).r);
}
