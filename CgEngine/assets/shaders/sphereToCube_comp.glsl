#version 450 core

layout(binding = 0) uniform sampler2D u_SphereMap;
layout(binding = 1, rgba32f) restrict writeonly uniform imageCube o_CubeMap;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

const float PI = 3.14159265359;

vec3 getCubeMapUVs() {
    vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(o_CubeMap));
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

void main() {
    vec3 cubeUV = getCubeMapUVs();

    float phi = atan(cubeUV.z, cubeUV.x);
    float theta = acos(cubeUV.y);
    vec2 uv = vec2(phi / (2.0 * PI) + 0.5, theta / PI);

    vec4 color = texture(u_SphereMap, uv);
    imageStore(o_CubeMap, ivec3(gl_GlobalInvocationID), color);
}
