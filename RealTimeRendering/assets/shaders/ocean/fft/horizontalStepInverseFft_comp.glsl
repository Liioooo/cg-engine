#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D precomputedData;
layout(rg32f, binding = 1) uniform image2D buffer0;
layout(rg32f, binding = 2) uniform image2D buffer1;

uniform bool pingPong;
uniform int _step;

#define PI 3.141592653589793

vec2 ComplexMult(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main() {
    uvec3 id = gl_GlobalInvocationID;
    vec4 data = imageLoad(precomputedData, ivec2(_step, id.x));
    uvec2 inputsIndices = uvec2(data.ba);
    if (pingPong) {
        vec2 value = imageLoad(buffer0, ivec2(inputsIndices.x, id.y)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(buffer0, ivec2(inputsIndices.y, id.y)).xy);
        imageStore(buffer1, ivec2(id.xy), vec4(value, 0, 1));
    } else {
        vec2 value = imageLoad(buffer1, ivec2(inputsIndices.x, id.y)).xy
        + ComplexMult(vec2(data.r, -data.g), imageLoad(buffer1, ivec2(inputsIndices.y, id.y)).xy);
        imageStore(buffer0, ivec2(id.xy), vec4(value, 0, 1));
    }
}