#ifndef UTILITIES_GLSL
#define UTILITIES_GLSL

float inverseLerp(float minValue, float maxValue, float v) {
    return (v - minValue) / (maxValue - minValue);
}

float remap(float v, float inMin, float inMax, float outMin, float outMax) {
    float t = inverseLerp(inMin, inMax, v);
    return mix(outMin, outMax, t);
}

float easeOut(float x, float t) {
    return 1.0 - pow(1.0 - x, t);
}

float easeIn(float x, float t) {
    return pow(x, t);
}

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0.0f, 0.0f),
        vec3(0.0f, c, -s),
        vec3(0.0f, s, c)
    );
}

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0.0f, s),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(-s, 0.0f, c)
    );
}

mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0.0f),
        vec3(s, c, 0.0f),
        vec3(0.0f, 0.0f, 1.0f)
    );
}

mat3 rotateAxis(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c
    );
}

#endif // UTILITIES_GLSL
