#version 450 core

layout(quads, fractional_odd_spacing, ccw) in;

#include "common/CameraDataBuffer.glsl"

in mat4 tcs_instanceTransform[];

out vec2 tessUV;

uniform mat4 u_Transform;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    float uMask = step(0.2f, u) - step(0.7f, u);
    float vMask = step(0.2f, v) - step(0.7f, v);
    float mask =  uMask * vMask;

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = ((p1 - p0) * v + p0); //+ vec4(0.0f, 0.0f, step(0.1f, mask) * u * 2.0f, 1.0f);

    vec4 worldPosition = u_Transform * tcs_instanceTransform[0] * p;

    gl_Position = u_CameraData.viewProjection * worldPosition;
    tessUV = vec2(gl_TessCoord.x, gl_TessCoord.y);
}

