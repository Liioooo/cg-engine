#version 450 core

#include "Macros.glsl"

layout(location = 10) in VS_OUT {
    vec3 TexCoord;
} fs_in;

layout (binding = 6) uniform samplerCube u_Texture;

PUSH_CONSTANT(SkyboxPC) {
    float intensity;
    float lod;
} pc_skybox;

layout(location = 0) out vec4 o_FragColor;

void main() {
    o_FragColor = clamp(textureLod(u_Texture, fs_in.TexCoord, pc_skybox.lod) * pc_skybox.intensity, vec4(0.0f), vec4(10.0f));
    o_FragColor.a = 1.0f;
}
