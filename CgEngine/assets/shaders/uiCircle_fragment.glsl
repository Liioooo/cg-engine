#version 450 core

in VS_OUT {
    vec2 TexCoord;
    vec4 LineColor;
    vec4 FillColor;
    float LineWidth;
    float Width;
} fs_in;

in flat float TextureIndex;

out vec4 o_FragColor;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main() {
    vec2 localPos = (fs_in.TexCoord - 0.5f) * 2.0f;
    float dist = length(localPos);

    if (dist > 1.0f) {
        discard;
    }

    vec4 fillColor = TextureIndex < 0.0f ? fs_in.FillColor : texture(u_Textures[int(TextureIndex)], fs_in.TexCoord);

    float lineAlpha = 1.0f - smoothstep(1.0f - fs_in.LineWidth / (fs_in.Width * 0.5f), 1.02f - fs_in.LineWidth / (fs_in.Width * 0.5f), dist);
    float fillAlpha = 1.0f - smoothstep(0.95f, 1.0f, dist);

    o_FragColor = vec4(mix(fs_in.LineColor.rgb, fillColor.rgb, lineAlpha), mix(fs_in.LineColor.a, fillColor.a, lineAlpha) * fillAlpha);

}
