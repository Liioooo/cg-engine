#ifndef NOISE_GLSL
#define NOISE_GLSL

#include "Hashing.glsl"

float noise11(float p) {
    float i = floor(p);

    float f = fract(p);
    float u = smoothstep(0.0, 1.0, f);

    float val = mix(hash11(i + 0.0),
    hash11(i + 1.0), u);
    return val * 2.0 - 1.0;
}

float noise12(vec2 p) {
    vec2 i = floor(p);

    vec2 f = fract(p);
    vec2 u = smoothstep(vec2(0.0), vec2(1.0), f);

    float val = mix( mix( hash12( i + vec2(0.0, 0.0) ),
    hash12( i + vec2(1.0, 0.0) ), u.x),
    mix( hash12( i + vec2(0.0, 1.0) ),
    hash12( i + vec2(1.0, 1.0) ), u.x), u.y);
    return val * 2.0 - 1.0;
}

float noise13(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    return mix(mix(mix( hash13(i+vec3(0.0, 0.0, 0.0)),
    hash13(i+vec3(1.0, 0.0, 0.0)),f.x),
    mix( hash13(i+vec3(0.0, 1.0, 0.0)),
    hash13(i+vec3(1.0, 1.0, 0.0)),f.x),f.y),
    mix(mix( hash13(i+vec3(0.0, 0.0, 1.0)),
    hash13(i+vec3(1.0, 0.0, 1.0)),f.x),
    mix( hash13(i+vec3(0.0, 1.0, 1.0)),
    hash13(i+vec3(1.0, 1.0, 1.0)),f.x),f.y),f.z);
}

vec2 noise23(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    return mix(mix(mix( hash23(i+vec3(0.0, 0.0, 0.0)),
    hash23(i+vec3(1.0, 0.0, 0.0)),f.x),
    mix( hash23(i+vec3(0.0, 1.0, 0.0)),
    hash23(i+vec3(1.0, 1.0, 0.0)),f.x),f.y),
    mix(mix( hash23(i+vec3(0.0, 0.0, 1.0)),
    hash23(i+vec3(1.0, 0.0, 1.0)),f.x),
    mix( hash23(i+vec3(0.0, 1.0, 1.0)),
    hash23(i+vec3(1.0, 1.0, 1.0)),f.x),f.y),f.z);
}

vec2 noise22(vec2 p) {
    vec2 i = floor(p);

    vec2 f = fract(p);
    vec2 u = smoothstep(vec2(0.0), vec2(1.0), f);

    vec2 val = mix( mix( hash22( i + vec2(0.0, 0.0) ),
    hash22( i + vec2(1.0, 0.0) ), u.x),
    mix( hash22( i + vec2(0.0, 1.0) ),
    hash22( i + vec2(1.0, 1.0) ), u.x), u.y);
    return val * 2.0 - 1.0;
}

#endif
