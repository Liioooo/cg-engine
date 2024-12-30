#version 450 core

#include "common/CameraDataBuffer.glsl"

layout (vertices=4) out;

uniform mat4 u_Transform;

in TS_OUT {
    vec4 aPos;
    vec4 aNormal;
    vec4 aTangent;
    vec4 aBitangent;
    vec4 aTexCoord;
} ts_in[];

out TS_OUT {
    vec4 aPos;
    vec4 aNormal;
    vec4 aTangent;
    vec4 aBitangent;
    vec4 aTexCoord;
} ts_out[];

void main()
{
    // ----------------------------------------------------------------------
    // pass attributes through
    ts_out[gl_InvocationID].aPos = ts_in[gl_InvocationID].aPos;
    ts_out[gl_InvocationID].aNormal = ts_in[gl_InvocationID].aNormal;
    ts_out[gl_InvocationID].aTangent = ts_in[gl_InvocationID].aTangent;
    ts_out[gl_InvocationID].aBitangent = ts_in[gl_InvocationID].aBitangent;
    ts_out[gl_InvocationID].aTexCoord = ts_in[gl_InvocationID].aTexCoord;

    // ----------------------------------------------------------------------
    // invocation zero controls tessellation levels for the entire patch
    if (gl_InvocationID == 0)
    {
        // Inspired from https://learnopengl.com/Guest-Articles/2021/Tessellation/Tessellation
        // ----------------------------------------------------------------------
        // Step 1: define constants to control tessellation parameters
        // set these as desired for your world scale
        const int MIN_TESS_LEVEL = 2;
        const int MAX_TESS_LEVEL = 64;
        const float MIN_DISTANCE = 10;
        const float MAX_DISTANCE = 130;

        float distance00 = clamp((length(vec3(u_CameraData.position - u_Transform * ts_in[0].aPos) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)), 0.0, 1.0);
        float distance01 = clamp((length(vec3(u_CameraData.position - u_Transform * ts_in[1].aPos) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)), 0.0, 1.0);
        float distance10 = clamp((length(vec3(u_CameraData.position - u_Transform * ts_in[2].aPos) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)), 0.0, 1.0);
        float distance11 = clamp((length(vec3(u_CameraData.position - u_Transform * ts_in[3].aPos) - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)), 0.0, 1.0);

        float tessLevel0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00));
        float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01));
        float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11));
        float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10));

        // ----------------------------------------------------------------------
        // Step 5: set the corresponding outer edge tessellation levels
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        // ----------------------------------------------------------------------
        // Step 6: set the inner tessellation levels to the max of the two parallel edges
        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}
	