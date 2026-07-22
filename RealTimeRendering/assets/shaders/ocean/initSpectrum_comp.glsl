#version 450 core

#include "Macros.glsl"

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const float TWO_PI = 6.283185307179586;
const float PI = 3.141592653589793;

UNIFORM_LAYOUT(0, 0) uniform sampler2D u_gaussianNoise;
UNIFORM_LAYOUT_FORMAT(1, 0, rgba32f) uniform image2D u_h0Texture;
UNIFORM_LAYOUT_FORMAT(2, 0, rgba32f) uniform image2D u_waveTexture;

layout (binding = 3, std140) uniform OceanData {
    float T;
    float gamma;
    float alpha;
    float omega_p;
    vec2 wind;
    int size;
    float _length;
    float depth;
    float g;
    float cutoffLow;
    float cutoffHigh;
} u_OceanData;

float frequencyDerivative(float k);
float omega(float k);
float JONSWAP(float k);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    int nx = texelCoord.x - u_OceanData.size / 2;
    int nz = texelCoord.y - u_OceanData.size / 2;
    float deltaK = TWO_PI / u_OceanData._length;
    vec2 k = vec2(TWO_PI * nx / u_OceanData._length, TWO_PI * nz / u_OceanData._length);
    float k_length = length(k);
    vec2 h0 = vec2(0.0, 0.0);
    vec4 wave = vec4(k.x, 1.0, k.y, 0.0);

    if (k_length <= u_OceanData.cutoffHigh && k_length >= u_OceanData.cutoffLow) {
        wave.g = 1.0 / k_length;
        wave.a = omega(k_length);
        float dOmegadk = frequencyDerivative(k_length);
        h0 = texelFetch(u_gaussianNoise, texelCoord, 0).xy * sqrt(2.0 * JONSWAP(k_length) * deltaK * deltaK * abs(dOmegadk) / k_length);
    }

    imageStore(u_h0Texture, texelCoord, vec4(h0.xy, 0.0, 1.0));
    imageStore(u_waveTexture, texelCoord, wave);
}

float frequencyDerivative(float k) {
    float th = tanh(min(k * u_OceanData.depth, 20.0));
    float ch = cosh(k * u_OceanData.depth);
    return u_OceanData.g * (u_OceanData.depth * k / ch / ch + th) / omega(k) / 2.0;
}

float omega(float k) {
    return sqrt(u_OceanData.g * k * tanh(min(k * u_OceanData.depth, 18.0)));
}

float JONSWAP(float k) {
    float sigma = 0.09;
    float omega = omega(k);
    if (omega < u_OceanData.omega_p) {
        sigma = 0.07;
    }
    float omega_recip = 1.0 / omega;
    float sigma_recip = 1.0 / sigma;
    float r = exp(-((omega - u_OceanData.omega_p) * (omega - u_OceanData.omega_p)) / (2.0 * sigma_recip * sigma_recip * u_OceanData.omega_p * u_OceanData.omega_p));
    float S = u_OceanData.alpha * u_OceanData.g * u_OceanData.g * omega_recip * omega_recip * omega_recip * omega_recip * omega_recip * exp(-1.25 * pow(u_OceanData.omega_p * omega_recip, 4.0)) * pow(u_OceanData.gamma, r);
    return S;
}
