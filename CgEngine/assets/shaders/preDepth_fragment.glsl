#version 450 core

#include "common/CameraDataBuffer.glsl"

in VS_OUT {
    vec3 Normal;
    mat3 CameraView;
} fs_in;

layout (location = 0) out vec3 o_ViewSpaceNormal;

void main() {
    o_ViewSpaceNormal = fs_in.CameraView * normalize(fs_in.Normal);
}
