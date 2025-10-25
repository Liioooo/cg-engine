#version 450 core

#include "CameraDataBuffer.glsl"
#include "Hashing.glsl"
#include "Noise.glsl"
#include "Utilities.glsl"
#include "PI.glsl"
#include "GBuffersVertex.glsl"

layout(binding = 6, std430) buffer Positions {
    vec2 positions[];
} b_Positions;

uniform layout(binding=10) sampler2D u_HeightGrassMap;

layout (binding = 5, std140) uniform CustomPipelineData {
    mat4 transform;
} u_CustomPipelineData;

layout (binding = 2, std140) uniform GrassData {
    vec2 grassParams; // x: GRASS_SEGMENTS, y: GRASS_VERTICES
    vec2 grassSize; // x: GRASS_WIDTH, y: GRASS_HEIGHT
    vec3 grassLOD; // x: GRASS_LOD_DIST, y: GRASS_MAX_DIST, z: terrainNormalRatio
    float time;
    vec3 islandCenter;
    vec2 islandSize;
    mat4 grassColor;
} u_GrassData;

layout (location = 0) in int vertId;

layout(location = 10) out VS_OUT {
    vec3 Normal0;
    vec3 Normal1;
    vec3 GrassParams; // x: heightPercent, y: xSide
    vec3 GrassColor;
} vs_out;

void main() {
    vec3 grassOffset = vec3(b_Positions.positions[gl_InstanceID].x, 0.0, b_Positions.positions[gl_InstanceID].y);
    vec3 grassBladeWorldPos = (u_CustomPipelineData.transform * vec4(grassOffset, 1.0)).xyz;

    vec2 islandCenterToGrassWorldPos = grassBladeWorldPos.xz - u_GrassData.islandCenter.xz;

    vec2 islandUV = vec2(0.5f, 0.5f) + vec2(islandCenterToGrassWorldPos.x * (1.0f / u_GrassData.islandSize.x), islandCenterToGrassWorldPos.y * (1.0f / u_GrassData.islandSize.y));

    vec2 heightGrassMapSample = textureLod(u_HeightGrassMap, islandUV, 0.0f).xy;

    vec4 hashVal = hash42(vec2(grassBladeWorldPos.x, grassBladeWorldPos.z));

    float highLODOut = smoothstep(u_GrassData.grassLOD.x * 0.5f, u_GrassData.grassLOD.x, distance(u_CameraData.position.xyz, grassBladeWorldPos));
    float highLODOutForTile = smoothstep(u_GrassData.grassLOD.x * 0.6f, u_GrassData.grassLOD.x, distance(u_CameraData.position.xyz, u_CustomPipelineData.transform[3].xyz));
    float lodFadeIn = smoothstep(u_GrassData.grassLOD.x, u_GrassData.grassLOD.y, distance(u_CameraData.position.xyz, grassBladeWorldPos));

    float randomAngle = hashVal.x * 2.0f * PI;
    float randomShade = remap(hashVal.y, -1.0f, 1.0f, 0.5f, 1.0f);
    float randomHeight = remap(hashVal.z, 0.0f, 1.0f, 0.75f, 1.5f) * mix(1.0f, 0.0f, lodFadeIn) * easeIn(heightGrassMapSample.y, 2.0f) * step(0.2f, heightGrassMapSample.y);
    float randomLean = remap(hashVal.w, 0.0f, 1.0f, 0.1f, 0.4f);

    vec2 hashGrassColour = hash22(vec2(grassBladeWorldPos.x, grassBladeWorldPos.z));

    float leanAnimation = noise12(vec2(u_GrassData.time * 0.35f) + grassBladeWorldPos.xz * 137.423f) * 0.1f;
    randomLean += leanAnimation;

    float GRASS_SEGMENTS = u_GrassData.grassParams.x;
    float GRASS_VERTICES = u_GrassData.grassParams.y;

    // Figure out vertex id, > GRASS_VERTICES is back side
    float vertID = mod(float(vertId), GRASS_VERTICES);

    // 1 = front, -1 = back
    float zSide = -(floor(vertId / GRASS_VERTICES) * 2.0f - 1.0f);

    // 0 = left, 1 = right
    float xSide = mod(vertID, 2.0f);

    float heightPercent = (vertID - xSide) / (GRASS_SEGMENTS * 2.0f);

    float heightLODFadeAdjust = mix(-heightPercent, 1.0f - heightPercent, step(0.5f, heightPercent));
    heightPercent += (heightLODFadeAdjust * highLODOutForTile);

    float grassTotalHeight = u_GrassData.grassSize.y * randomHeight;

    float grassTotalWidthHigh = easeOut(1.0f - heightPercent, 2.0f);
    float grassTotalWidthLow = 1.0f - heightPercent;
    float grassTotalWidth = u_GrassData.grassSize.x * mix(grassTotalWidthHigh, grassTotalWidthLow, highLODOut) * easeIn(heightGrassMapSample.y, 1.5f) * step(0.2f, heightGrassMapSample.y);

    float x = (xSide - 0.5) * grassTotalWidth;
    float y = heightPercent * grassTotalHeight;

    float windDir = noise12(grassBladeWorldPos.xz * 0.05f + 0.05f * u_GrassData.time);
    float windNoiseSample = noise12(grassBladeWorldPos.xz * 0.25f + u_GrassData.time * 1.0f);
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

    vec2 texelSizeHeightMap = 1.0f / textureSize(u_HeightGrassMap, 0);

    float heightL = texture(u_HeightGrassMap, islandUV - vec2(texelSizeHeightMap.x, 0.0f)).r;
    float heightR = texture(u_HeightGrassMap, islandUV + vec2(texelSizeHeightMap.x, 0.0f)).r;
    float heightD = texture(u_HeightGrassMap, islandUV - vec2(0.0f, texelSizeHeightMap.y)).r;
    float heightU = texture(u_HeightGrassMap, islandUV + vec2(0.0f, texelSizeHeightMap.y)).r;

    // Compute gradients
    float dX = heightR - heightL; // Gradient in X
    float dZ = heightU - heightD; // Gradient in Z

    vec3 terrainNormal = normalize(vec3(-dX, 1.0f, -dZ));

    float skyFadeIn = (1.0f - highLODOut) * u_GrassData.grassLOD.z;
    grassVertexNormal0 = normalize(mix(terrainNormal, normalize(grassVertexNormal0), skyFadeIn));
    grassVertexNormal1 = normalize(mix(terrainNormal, normalize(grassVertexNormal1), skyFadeIn));

    vec3 grassVertexPosition = vec3(x, y, 0.0f);
    grassVertexPosition = grassMat * grassVertexPosition;
    grassVertexPosition += grassBladeWorldPos;
    grassVertexPosition.y += heightGrassMapSample.x * 50.0f;

    vec3 b1 = u_GrassData.grassColor[0].rgb;
    vec3 b2 = u_GrassData.grassColor[1].rgb;
    vec3 t1 = u_GrassData.grassColor[2].rgb;
    vec3 t2 = u_GrassData.grassColor[3].rgb;

    vec3 baseColour = mix(b1, b2, hashGrassColour.x);
    vec3 tipColour = mix(t1, t2, hashGrassColour.y);
    vec3 highLODColour = mix(baseColour, tipColour, easeIn(heightPercent, 4.0f)) * randomShade;
    vec3 lowLODColour = mix(b1, t1, heightPercent);
    vs_out.GrassColor = mix(highLODColour, lowLODColour, highLODOut);

    vs_out.Normal0 = grassVertexNormal0;
    vs_out.Normal1 = grassVertexNormal1;
    vs_out.GrassParams = vec3(heightPercent, xSide, highLODOut);

    gl_Position = u_CameraData.viewProjection * vec4(grassVertexPosition, 1.0f);

    passGBufferData();
}
