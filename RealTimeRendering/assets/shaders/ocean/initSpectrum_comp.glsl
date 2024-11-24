#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const float TWO_PI = 6.283185307179586;
const float PI = 3.141592653589793;

layout(rg32f, binding = 0) uniform image2D u_gaussianNoise;
layout(rgba32f, binding = 1) uniform image2D u_h0Texture;
layout(rgba32f, binding = 2) uniform image2D u_waveTexture;

uniform float u_T;
uniform float u_gamma;
uniform float u_alpha;
uniform float u_omega_p;
uniform vec2 u_wind;
uniform int u_size;
uniform float u_length;
uniform float u_depth;
uniform float u_g;
uniform float u_cutoffLow = 5.0;
uniform float u_cutoffHigh = TWO_PI / 17.0 * 6.0;
float omega_0 = TWO_PI / u_T;

float frequencyDerivative(float k);
float omega(float k);
float JONSWAP(vec2 k);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    int nx = texelCoord.x - u_size / 2;
    int nz = texelCoord.y - u_size / 2;
    float deltaK = TWO_PI / u_length;
    vec2 k = vec2(TWO_PI * nx / u_length, TWO_PI * nz / u_length);
    float k_length = length(k);
    vec2 h0 = vec2(0, 0);
    vec4 wave = vec4(k.x, 1, k.y, 0);

    if (k_length <= u_cutoffHigh && k_length >= u_cutoffLow) {
        wave.g = 1 / k_length;
        wave.a = omega(k_length);
        float dOmegadk = frequencyDerivative(k_length);
        h0 = imageLoad(u_gaussianNoise, ivec2(gl_GlobalInvocationID.xy)).xy * sqrt(2.0 * JONSWAP(k) * deltaK * deltaK * abs(dOmegadk) / k_length);
    }

    imageStore(u_h0Texture, texelCoord, vec4(h0.xy, 0, 1));
    imageStore(u_waveTexture, texelCoord, wave);
}

float frequencyDerivative(float k) {
    float th = tanh(min(k * u_depth, 20));
    float ch = cosh(k * u_depth);
    return u_g * (u_depth * k / ch / ch + th) / omega(k) / 2;
}

float omega(float k) {
    return sqrt(u_g * k * tanh(min(k * u_depth, 18)));
}

float JONSWAP(vec2 k) {
    float sigma = 0.09;
    float omega = omega(length(k));
    if (omega < u_omega_p) {
        sigma = 0.07;
    }
    float omega_recip = 1.0 / omega;
    float sigma_recip = 1.0 / sigma;
    float r = exp(-((omega - u_omega_p) * (omega - u_omega_p)) / (2.0 * sigma_recip * sigma_recip * u_omega_p * u_omega_p));
    float S = u_alpha * u_g * u_g * omega_recip * omega_recip * omega_recip * omega_recip * omega_recip * exp(-1.25 * pow(u_omega_p * omega_recip, 4)) * pow(u_gamma, r);
    return S;
}
