#version 450 core

#include "TransformsOffsetPC.glsl"
#include "Macros.glsl"

layout(binding = 0, std430) buffer Transforms {
    mat4 transforms[];
} b_Transforms;

layout (location = 0) in vec4 a_Pos;
layout (location = 1) in vec4 a_Normal;
layout (location = 2) in vec4 a_Tangent;
layout (location = 3) in vec4 a_Bitangent;
layout (location = 4) in vec4 a_TexCoord;

void main() {
    mat4 model = b_Transforms.transforms[pc_transformsOffset.transformsOffset + GET_INSTANCE_INDEX()];
    gl_Position = model * a_Pos;
}
