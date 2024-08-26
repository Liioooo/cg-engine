#version 450 core

in VS_OUT {
    vec2 TexCoord;
    vec4 LineColor;
    vec4 FillColor;
    vec2 Size;
    float LineWidth;
} fs_in;

in flat float TextureIndex;

out vec4 o_FragColor;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main() {
    vec2 localPos = (fs_in.TexCoord - 0.5f) * 2.0f;
    float adjustedLineWidth = fs_in.LineWidth + 0.5f;

    vec4 fillColor = TextureIndex < 0.0f ? fs_in.FillColor : texture(u_Textures[int(TextureIndex)], fs_in.TexCoord);

    float line = fs_in.LineWidth > 0.0f ? min(step(fs_in.Size.x - adjustedLineWidth, fs_in.Size.x * abs(localPos.x)) + step(fs_in.Size.y - adjustedLineWidth, fs_in.Size.y * abs(localPos.y)), 1.0f) : 0.0f;
    o_FragColor = mix(fillColor, fs_in.LineColor, line);
}
