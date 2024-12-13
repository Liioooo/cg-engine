layout (location = 0) out vec4 o_AlbedoRoughness;
layout (location = 1) out vec4 o_EmissionMetalic;
layout (location = 2) out vec3 o_WorldNormal;
layout (location = 3) out vec3 o_ViewNormal;

in GBuffers_OUT {
    mat3 CameraView;
} fs_in_gBuffers;

void outputToGBuffers(vec3 albedoColor, float roughness, vec3 emission, float metalness, vec3 normalMappedNormal, vec3 vertexNormal) {
    o_AlbedoRoughness = vec4(albedoColor, roughness);
    o_EmissionMetalic = vec4(emission, metalness);
    o_WorldNormal = normalMappedNormal;
    o_ViewNormal = fs_in_gBuffers.CameraView * vertexNormal;
}
