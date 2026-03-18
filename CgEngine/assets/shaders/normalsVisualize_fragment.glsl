#version 450 core

layout(early_fragment_tests) in;

#include "Macros.glsl"

PUSH_CONSTANT(CollidersPC) {
    vec3 color;
    int transformsOffset;
} pc_colliders;

layout(location = 0) out vec4 o_FragColor;

void main() {
    o_FragColor = vec4(pc_colliders.color.rgb, 1.0f);
}
