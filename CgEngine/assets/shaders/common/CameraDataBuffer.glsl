layout (binding = 0, std140) uniform CameraData {
    mat4 viewProjection;
    mat4 projection;
    mat4 view;
    mat4 uiProjectionMatrix;
    vec4 position;
    vec4 clipInfo; // z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0
    float exposure;
    float bloomIntensity;
    float bloomThreshold;
    float _padding_; // Needed because on some drivers this is added automatically and on some not...
} u_CameraData;
