#version 450 core

#include "common/CameraDataBuffer.glsl"
#include "common/Hashing.glsl"
#include "common/Noise.glsl"
#include "common/Utilities.glsl"
#include "common/PI.glsl"

layout(binding = 5, std430) buffer Positions {
    vec2 positions[];
} b_Positions;

uniform mat4 u_Transform;
uniform vec2 u_GrassParams; // x: GRASS_SEGMENTS, y: GRASS_VERTICES
uniform vec2 u_GrassSize; // x: GRASS_WIDTH, y: GRASS_HEIGHT
uniform vec2 u_GrassLOD; // x: GRASS_LOD_DIST, y: GRASS_MAX_DIST
uniform float u_Time;

layout (location = 0) in int vertId;

out VS_OUT {
    vec3 WorldPosition;
    vec3 Normal0;
    vec3 Normal1;
    vec3 GrassParams; // x: heightPercent, y: xSide
    vec3 GrassColor;
} vs_out;

void main() {
    vec3 grassOffset = vec3(b_Positions.positions[gl_InstanceID].x, 0.0, b_Positions.positions[gl_InstanceID].y);
    vec3 grassBladeWorldPos = (u_Transform * vec4(grassOffset, 1.0)).xyz;

    vec4 hashVal = hash42(vec2(grassBladeWorldPos.x, grassBladeWorldPos.z));

    float highLODOut = smoothstep(u_GrassLOD.x * 0.5, u_GrassLOD.x, distance(u_CameraData.position.xyz, grassBladeWorldPos));
    float lodFadeIn = smoothstep(u_GrassLOD.x, u_GrassLOD.y, distance(u_CameraData.position.xyz, grassBladeWorldPos));

    float isGrassAllowed = 1.0f;

    float randomAngle = hashVal.x * 2.0f * PI;
    float randomShade = remap(hashVal.y, -1.0f, 1.0f, 0.5f, 1.0f);
    float randomHeight = remap(hashVal.z, 0.0f, 1.0f, 0.75f, 1.5f) * mix(1.0f, 0.0f, lodFadeIn) * isGrassAllowed;
    float randomLean = remap(hashVal.w, 0.0f, 1.0f, 0.1f, 0.4f);

    vec2 hashGrassColour = hash22(vec2(grassBladeWorldPos.x, grassBladeWorldPos.z));

    float leanAnimation = noise12(vec2(u_Time * 0.35f) + grassBladeWorldPos.xz * 137.423f) * 0.1f;
    randomLean += leanAnimation;

    float GRASS_SEGMENTS = u_GrassParams.x;
    float GRASS_VERTICES = u_GrassParams.y;

    // Figure out vertex id, > GRASS_VERTICES is back side
    float vertID = mod(float(vertId), GRASS_VERTICES);

    // 1 = front, -1 = back
    float zSide = -(floor(vertId / GRASS_VERTICES) * 2.0f - 1.0f);

    // 0 = left, 1 = right
    float xSide = mod(vertID, 2.0f);

    float heightPercent = (vertID - xSide) / (GRASS_SEGMENTS * 2.0f);

    float grassTotalHeight = u_GrassSize.y * randomHeight;

    float grassTotalWidthHigh = easeOut(1.0f - heightPercent, 2.0f);
    float grassTotalWidthLow = 1.0f - heightPercent;
    float grassTotalWidth = u_GrassSize.x * mix(grassTotalWidthHigh, grassTotalWidthLow, highLODOut);

    float x = (xSide - 0.5) * grassTotalWidth;
    float y = heightPercent * grassTotalHeight;

    float windDir = noise12(grassBladeWorldPos.xz * 0.05f + 0.05f * u_Time);
    float windNoiseSample = noise12(grassBladeWorldPos.xz * 0.25f + u_Time * 1.0f);
    float windLeanAngle = remap(windNoiseSample, -1.0f, 1.0f, 0.25f, 1.0f);
    windLeanAngle = easeIn(windLeanAngle, 2.0f) * 1.25f;
    vec3 windAxis = vec3(cos(windDir), 0.0f, sin(windDir));
    windLeanAngle *= heightPercent;

    float easedHeight = mix(easeIn(heightPercent, 2.0), 1.0, highLODOut);
    float curveAmount = -randomLean * easedHeight;

    mat3 grassMat =  rotateAxis(windAxis, windLeanAngle) * rotateY(randomAngle) * rotateX(curveAmount);

    vec3 grassVertexNormal = vec3(0.0f, 0.0f, 1.0f);
    vec3 grassVertexNormal0 = rotateY(PI * 0.3f * zSide) * grassVertexNormal;
    vec3 grassVertexNormal1 = rotateY(PI * -0.3f * zSide) * grassVertexNormal;

    grassVertexNormal0 = grassMat * grassVertexNormal0;
    grassVertexNormal0 *= zSide;

    grassVertexNormal1 = grassMat * grassVertexNormal1;
    grassVertexNormal1 *= zSide;

    vec3 grassVertexPosition = vec3(x, y, 0.0f);
    grassVertexPosition = grassMat * grassVertexPosition;
    grassVertexPosition += grassBladeWorldPos;

    vec3 b1 = vec3(0.02, 0.075, 0.01);
    vec3 b2 = vec3(0.025, 0.1, 0.01);
    vec3 t1 = vec3(0.65, 0.8, 0.25);
    vec3 t2 = vec3(0.8, 0.9, 0.4);

    vec3 baseColour = mix(b1, b2, hashGrassColour.x);
    vec3 tipColour = mix(t1, t2, hashGrassColour.y);
    vec3 highLODColour = mix(baseColour, tipColour, easeIn(heightPercent, 4.0f)) * randomShade;
    vec3 lowLODColour = mix(b1, t1, heightPercent);
    vs_out.GrassColor = mix(highLODColour, lowLODColour, highLODOut);

    mat3 normalMat = mat3(transpose(inverse(u_Transform)));

    vs_out.WorldPosition = grassVertexPosition;
    vs_out.Normal0 = normalMat * grassVertexNormal0;
    vs_out.Normal1 = normalMat * grassVertexNormal1;
    vs_out.GrassParams = vec3(heightPercent, xSide, highLODOut);

    gl_Position = u_CameraData.viewProjection * vec4(grassVertexPosition, 1.0f);
}
