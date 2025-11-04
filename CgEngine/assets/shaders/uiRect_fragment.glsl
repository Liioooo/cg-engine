#version 450 core

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
    vec4 LineColor;
    vec4 FillColor;
    vec2 Size;
    float LineWidth;
} fs_in;

layout(location = 20) in flat float TextureIndex;

layout(location = 0) out vec4 o_FragColor;

layout(binding = 0) uniform sampler2D u_Textures[16];

vec4 toLinearRGB(vec4 color) {
    return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
    vec2 localPos = (fs_in.TexCoord - 0.5f) * 2.0f;
    float adjustedLineWidth = fs_in.LineWidth + 0.5f;

    vec4 fillColor = TextureIndex < 0.0f ? toLinearRGB(fs_in.FillColor) : texture(u_Textures[int(TextureIndex)], fs_in.TexCoord);

    float line = fs_in.LineWidth > 0.0f ? min(step(fs_in.Size.x - adjustedLineWidth, fs_in.Size.x * abs(localPos.x)) + step(fs_in.Size.y - adjustedLineWidth, fs_in.Size.y * abs(localPos.y)), 1.0f) : 0.0f;
    o_FragColor = mix(fillColor, toLinearRGB(fs_in.LineColor), line);
}
