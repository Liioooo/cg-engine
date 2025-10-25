#version 450 core

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

layout(location = 10) out VS_OUT_TO_TCS {
    vec4 aPos;
    vec4 aNormal;
    vec4 aTangent;
    vec4 aBitangent;
    vec4 aTexCoord;
} vs_out_to_tcs;

void main() {
    vs_out_to_tcs.aPos = a_Pos;
    vs_out_to_tcs.aNormal = a_Normal;
    vs_out_to_tcs.aTangent = a_Tangent;
    vs_out_to_tcs.aBitangent = a_Bitangent;
    vs_out_to_tcs.aTexCoord = a_TexCoord;
}

