layout (binding = 0, std140) uniform CameraData {
    mat4 viewProjection;
    mat4 projection;
    mat4 view;
    mat4 uiProjectionMatrix;
    vec4 position;
    float exposure;
    float bloomIntensity;
    float bloomThreshold;
} u_CameraData;