layout (binding = 2, std140) uniform DirShadowData {
    mat4 lightSpaceMat[4];
    vec4 cascadeSplits;
} u_DirShadowData;

vec4[4] calcDirShadowMapPostion(vec3 worldPosition) {
    vec4[4] dirShadowMapPosition;

    dirShadowMapPosition[0] = u_DirShadowData.lightSpaceMat[0] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[1] = u_DirShadowData.lightSpaceMat[1] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[2] = u_DirShadowData.lightSpaceMat[2] * vec4(worldPosition, 1.0f);
    dirShadowMapPosition[3] = u_DirShadowData.lightSpaceMat[3] * vec4(worldPosition, 1.0f);

    return dirShadowMapPosition;
}
