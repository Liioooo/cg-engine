#version 450 core

#include "Macros.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rg32f, binding = 0) uniform restrict readonly image2D u_DxDz;
layout(rg32f, binding = 1) uniform restrict readonly image2D u_DyDxz;
layout(rg32f, binding = 2) uniform restrict readonly image2D u_DyxDyz;
layout(rg32f, binding = 3) uniform restrict readonly image2D u_DxxDzz;
layout(rgba32f, binding = 4) uniform restrict writeonly image2D u_displacement;
layout(rgba32f, binding = 5) uniform restrict writeonly image2D u_derivatives;
layout(rgba32f, binding = 6) uniform restrict image2D u_turbulence;

PUSH_CONSTANT(PCFinalTextures, 10) {
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
