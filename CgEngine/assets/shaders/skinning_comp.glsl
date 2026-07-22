#version 450 core

#include "Macros.glsl"

const int MAX_BONES = 200;
const int MAX_ANIMATED_COMPONENTS = 512;

struct BoneInfluence {
    uvec4 boneIndices;
    vec4 weights;
};

UNIFORM_LAYOUT_STD430(1, 1) readonly buffer BoneInfluences {
    BoneInfluence boneInfluences[];
} b_BoneInfluences;

UNIFORM_LAYOUT_STD430(2, 0) readonly buffer BoneTransforms {
    mat4 boneTransforms[MAX_BONES * MAX_ANIMATED_COMPONENTS];
} b_BoneTransforms;

struct Vertex {
    vec4 pos;
    vec4 normal;
    vec4 tangent;
    vec4 bitangent;
    vec4 texCoord;
};

UNIFORM_LAYOUT_STD430(3, 1) readonly buffer VertexBufferIn {
    Vertex vertices[];
} b_VertexBufferIn;

UNIFORM_LAYOUT_STD430(4, 1) writeonly buffer VertexBufferOut {
    Vertex vertices[];
} b_VertexBufferOut;

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

PUSH_CONSTANT(SkinningPC) {
    int componentIndex;
} pc_skinning;

void main() {
    BoneInfluence boneInfluence = b_BoneInfluences.boneInfluences[gl_GlobalInvocationID.x];

    mat4 boneTransform = b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[0] + pc_skinning.componentIndex * MAX_BONES] * boneInfluence.weights[0];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[1] + pc_skinning.componentIndex * MAX_BONES] * boneInfluence.weights[1];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[2] + pc_skinning.componentIndex * MAX_BONES] * boneInfluence.weights[2];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[3] + pc_skinning.componentIndex * MAX_BONES] * boneInfluence.weights[3];

    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].pos = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].pos;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].normal = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].normal;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].tangent = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].tangent;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].bitangent = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].bitangent;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].texCoord = b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].texCoord;
}
