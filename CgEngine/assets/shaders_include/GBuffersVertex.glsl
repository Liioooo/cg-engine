#ifndef G_BUFFERS_VERTEX_GLSL
#define G_BUFFERS_VERTEX_GLSL

#include "CameraDataBuffer.glsl"

layout(location = 30) out GBuffers_OUT {
    mat3 CameraView;
} vs_out_gBuffers;

void passGBufferData() {
    vs_out_gBuffers.CameraView = mat3(u_CameraData.view);
}

#endif // G_BUFFERS_VERTEX_GLSL
