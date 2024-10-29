#version 450 core

#include "common/CameraDataBuffer.glsl"

in VS_OUT {
    vec3 Normal;
} fs_in;

layout (location = 0) out vec3 o_ViewSpaceNormal;

void main() {
    o_ViewSpaceNormal = (u_CameraData.view * vec4(normalize(fs_in.Normal), 1.0f)).xyz;
}
