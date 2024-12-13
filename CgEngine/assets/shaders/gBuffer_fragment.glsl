#version 450 core

#include "common/PBRMaterialUniforms.glsl"
#include "common/GBuffersFragment.glsl"

in VS_OUT {
    vec2 TexCoord;
    mat3 TBN;
    vec3 Normal;
} fs_in;

void main() {
    vec3 mat_AlbedoColor = texture(u_Mat_AlbedoTexture, fs_in.TexCoord).rgb * u_Mat_AlbedoColor;
    float mat_Metalness = texture(u_Mat_MetalnessTexture, fs_in.TexCoord).r * u_Mat_Metalness;
    float mat_Roughness = texture(u_Mat_RoughnessTexture, fs_in.TexCoord).r * u_Mat_Roughness;
    vec3 mat_Emission = texture(u_Mat_EmissionTexture, fs_in.TexCoord).rgb * u_Mat_Emission;

    vec3 vertexNormal = normalize(fs_in.Normal);
    vec3 mat_Normal = vertexNormal;
    if (u_Mat_UseNormals) {
        mat_Normal = normalize(texture(u_Mat_NormalTexture, fs_in.TexCoord).rgb * 2.0f - 1.0f);
        mat_Normal = normalize(fs_in.TBN * mat_Normal);
    }

    outputToGBuffers(mat_AlbedoColor, mat_Roughness, mat_Emission, mat_Metalness, mat_Normal, vertexNormal);
}
