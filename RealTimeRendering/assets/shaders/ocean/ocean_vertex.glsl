#version 450 core

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

layout(location = 10) out TS_OUT {
    vec4 aPos;
    vec4 aNormal;
    vec4 aTangent;
    vec4 aBitangent;
    vec4 aTexCoord;
} ts_out;

void main() {
    ts_out.aPos = a_Pos;
    ts_out.aNormal = a_Normal;
    ts_out.aTangent = a_Tangent;
    ts_out.aBitangent = a_Bitangent;
    ts_out.aTexCoord = a_TexCoord;
}

