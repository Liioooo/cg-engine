#version 450 core

const int MAX_BONES = 100;
const int MAX_ANIMATED_COMPONENTS = 512;

struct BoneInfluence {
    uvec4 boneIndices;
    vec4 weights;
};

layout(binding = 1, std430) readonly buffer BoneInfluences {
    BoneInfluence boneInfluences[];
} b_BoneInfluences;

layout(binding = 2, std430) readonly buffer BoneTransforms {
    mat4 boneTransforms[MAX_BONES * MAX_ANIMATED_COMPONENTS];
} b_BoneTransforms;

struct Vertex {
    vec4 pos;
    vec4 normal;
    vec4 tangent;
    vec4 bitangent;
    vec4 texCoord;
};

layout(binding = 3, std430) readonly buffer VertexBufferIn {
    Vertex vertices[];
} b_VertexBufferIn;

layout(binding = 4, std430) writeonly buffer VertexBufferOut {
    Vertex vertices[];
} b_VertexBufferOut;

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uniform int u_ComponentIndex;

void main() {
    BoneInfluence boneInfluence = b_BoneInfluences.boneInfluences[gl_GlobalInvocationID.x];

    mat4 boneTransform = b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[0] + u_ComponentIndex * MAX_BONES] * boneInfluence.weights[0];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[1] + u_ComponentIndex * MAX_BONES] * boneInfluence.weights[1];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[2] + u_ComponentIndex * MAX_BONES] * boneInfluence.weights[2];
    boneTransform += b_BoneTransforms.boneTransforms[boneInfluence.boneIndices[3] + u_ComponentIndex * MAX_BONES] * boneInfluence.weights[3];

    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].pos = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].pos;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].normal = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].normal;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].tangent = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].tangent;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].bitangent = boneTransform * b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].bitangent;
    b_VertexBufferOut.vertices[gl_GlobalInvocationID.x].texCoord = b_VertexBufferIn.vertices[gl_GlobalInvocationID.x].texCoord;
}