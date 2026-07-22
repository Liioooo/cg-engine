#version 450 core

#include "Macros.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

UNIFORM_LAYOUT_FORMAT(0, 0, rg32f) uniform restrict readonly image2D u_DxDz;
UNIFORM_LAYOUT_FORMAT(1, 0, rg32f) uniform restrict readonly image2D u_DyDxz;
UNIFORM_LAYOUT_FORMAT(2, 0, rg32f) uniform restrict readonly image2D u_DyxDyz;
UNIFORM_LAYOUT_FORMAT(3, 0, rg32f) uniform restrict readonly image2D u_DxxDzz;
UNIFORM_LAYOUT_FORMAT(4, 0, rgba32f) uniform restrict writeonly image2D u_displacement;
UNIFORM_LAYOUT_FORMAT(5, 0, rgba32f) uniform restrict writeonly image2D u_derivatives;
UNIFORM_LAYOUT_FORMAT(6, 0, rgba32f) uniform restrict image2D u_turbulence;

PUSH_CONSTANT(PCFinalTextures) {
    float deltaTime;
    float lambda;
} pc_finalTextures;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 dxDz = imageLoad(u_DxDz, texelCoord).xy;
    vec2 dyDxz = imageLoad(u_DyDxz, texelCoord).xy;
    vec2 dyxDyz = imageLoad(u_DyxDyz, texelCoord).xy;
    vec2 dxxDzz = imageLoad(u_DxxDzz, texelCoord).xy;

    imageStore(u_displacement, texelCoord, vec4(pc_finalTextures.lambda * dxDz.x, dyDxz.x, pc_finalTextures.lambda * dxDz.y, 0));
    imageStore(u_derivatives, texelCoord, vec4(dyxDyz, dxxDzz * pc_finalTextures.lambda));
    float jacobian = (1 + pc_finalTextures.lambda * dxxDzz.x) * (1 + pc_finalTextures.lambda * dxxDzz.y) - pc_finalTextures.lambda * pc_finalTextures.lambda * dyDxz.y * dyDxz.y;
    float turbulenceR = imageLoad(u_turbulence, texelCoord).r + pc_finalTextures.deltaTime * 0.5 / max(jacobian, 0.5);
    imageStore(u_turbulence, texelCoord, vec4(min(turbulenceR, jacobian), 0.0, 0.0, 1.0));
}
