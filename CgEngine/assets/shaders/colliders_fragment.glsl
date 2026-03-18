#version 450 core

#include "Macros.glsl"

layout(location = 0) out vec4 o_FragColor;

PUSH_CONSTANT(CollidersPC) {
    vec3 color;
    int transformsOffset;
} pc_colliders;

void main() {
    o_FragColor = vec4(pc_colliders.color.rgb, 1.0f);
}
