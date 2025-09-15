#version 450 core

#include "PBRMaterialUniforms.glsl"
#include "GBuffersFragment.glsl"

layout(location = 10) in VS_OUT {
    vec2 TexCoord;
    mat3 TBN;
    vec3 Normal;
} fs_in;

void main() {
    vec3 mat_AlbedoColor = texture(u_Mat_AlbedoTexture, fs_in.TexCoord).rgb * pc_material.albedoColor;
    float mat_Metalness = texture(u_Mat_MetalnessTexture, fs_in.TexCoord).r * pc_material.metalness;
    float mat_Roughness = texture(u_Mat_RoughnessTexture, fs_in.TexCoord).r * pc_material.roughness;
    vec3 mat_Emission = texture(u_Mat_EmissionTexture, fs_in.TexCoord).rgb * pc_material.emission;

    vec3 vertexNormal = normalize(fs_in.Normal);
    vec3 mat_Normal = vertexNormal;
    if (pc_material.useNormals) {
        mat_Normal = normalize(texture(u_Mat_NormalTexture, fs_in.TexCoord).rgb * 2.0f - 1.0f);
        mat_Normal = normalize(fs_in.TBN * mat_Normal);
    }

    outputToGBuffers(mat_AlbedoColor, mat_Roughness, mat_Emission, mat_Metalness, mat_Normal, vertexNormal);
}
