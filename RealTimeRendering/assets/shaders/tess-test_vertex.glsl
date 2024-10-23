#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (location = 0) in vec2 a_Pos;

layout(binding = 5, std430) buffer InstanceBuffer {
    mat4 transforms[];
} b_InstanceBuffer;

out mat4 vs_instanceTransform;

void main() {
    gl_Position = vec4(a_Pos.xy, 0.0f, 1.0f);
    vs_instanceTransform = b_InstanceBuffer.transforms[gl_InstanceID];
}

