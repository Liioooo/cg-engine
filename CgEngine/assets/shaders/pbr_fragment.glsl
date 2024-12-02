#version 450 core

#include "common/PBRMaterialUniforms.glsl"
#include "common/CameraDataBuffer.glsl"
#include "common/LightDataBuffer.glsl"
#include "common/DirShadowMappingFragment.glsl"
#include "common/DirLightCalculationsFragment.glsl"
#include "common/PointLightsCalculationsFragment.glsl"
#include "common/SpotLightsCalculationsFragment.glsl"
#include "common/IBLCalculationsFragment.glsl"
#include "common/LightCalculationsHelperFragment.glsl"
#include "common/HBAOSampling.glsl"
#include "common/ScreenDataBuffer.glsl"

layout(early_fragment_tests) in;

in VS_OUT {
    vec3 WorldPosition;
    vec4 DirShadowMapPosition[4];
    vec3 Normal;
    mat3 TBN;
    vec2 TexCoord;
} fs_in;

out vec4 o_FragColor;

void main() {
    vec3 mat_AlbedoColor = texture(u_Mat_AlbedoTexture, fs_in.TexCoord).rgb * u_Mat_AlbedoColor;
    float mat_Metalness = texture(u_Mat_MetalnessTexture, fs_in.TexCoord).r * u_Mat_Metalness;
    float mat_Roughness = texture(u_Mat_RoughnessTexture, fs_in.TexCoord).r * u_Mat_Roughness;
    vec3 mat_Emission = texture(u_Mat_EmissionTexture, fs_in.TexCoord).rgb * u_Mat_Emission;

    vec3 mat_Normal = normalize(fs_in.Normal);
    if (u_Mat_UseNormals) {
        mat_Normal = normalize(texture(u_Mat_NormalTexture, fs_in.TexCoord).rgb * 2.0f - 1.0f);
        mat_Normal = normalize(fs_in.TBN * mat_Normal);
    }

    vec3 V = normalize(u_CameraData.position.xyz - fs_in.WorldPosition);
    float NdotV = max(dot(mat_Normal, V), 0.0f);

    vec3 F0 = calcF0(mat_AlbedoColor, mat_Metalness);

    vec3 light = calcDirLight(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * (1.0f - calcDirShadow(mat_Normal, u_LightData.dirLightDirection.xyz, u_CameraData.view, fs_in.WorldPosition, fs_in.DirShadowMapPosition));
    light += calcPointLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, fs_in.WorldPosition, mat_Normal, V, NdotV);
    light += calcSpotLights(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, fs_in.WorldPosition, mat_Normal, V, NdotV);
    light += mat_Emission;

    vec3 ibl = calcIBL(F0, mat_AlbedoColor, mat_Metalness, mat_Roughness, mat_Normal, V, NdotV) * u_EnvironmentIntensity * texture(u_HBAO_Tex, gl_FragCoord.xy * u_ScreenData.invFullResolution).r;

    o_FragColor = vec4(light + ibl, 1.0f);


//    vec4 viewSpacePos = u_CameraData.view * vec4(fs_in.WorldPosition, 1.0f);
//    switch (getShadowCascade(abs(viewSpacePos.z)))
//    {
//        case 0:
//        o_FragColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
//        break;
//        case 1:
//        o_FragColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
//        break;
//        case 2:
//        o_FragColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
//        break;
//        case 3:
//        o_FragColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
//        break;
//    }
}
