#include "common/CameraDataBuffer.glsl"

out GBuffers_OUT {
    mat3 CameraView;
} vs_out_gBuffers;

void passGBufferData() {
    vs_out_gBuffers.CameraView = mat3(u_CameraData.view);
}
