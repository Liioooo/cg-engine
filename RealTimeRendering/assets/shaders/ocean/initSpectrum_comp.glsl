#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rg32f, binding = 0) uniform image2D gaussianNoise;
layout(rgba32f, binding = 1) uniform image2D h0_texture;
layout(rgba32f, binding = 2) uniform image2D wave_texture;

const float TWO_PI = 6.283185307179586;
ivec2 L = ivec2(100, 100);
int N = 256;
int M = 256;
int V = 11;
int l = 1;
float g = 9.81;
float U_10 = 15;
uniform vec2 omega_hat = vec2(0.2, 0.4);
float T = 20.0;
float omega_0 = TWO_PI / T;
float D = 500.0;

float omega(float k);
float omega_bar(float k);
vec2 h_tilde_0(vec2 k);
float P_h(vec2 k);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    int nx = texelCoord.x - N / 2;
    int nz = texelCoord.y - M / 2;
    vec2 k = vec2(TWO_PI * nx / L.x, TWO_PI * nz / L.y);
    float k_length = length(k);
    vec2 h0 = h_tilde_0(k);
    vec4 wave = vec4(k.x, 1, k.y, 0);

    if (k_length <= 2 && k_length >= 0.0001) {
        wave.g = 1 / k_length;
        wave.a = omega_bar(k_length);
    } else {
    }

    imageStore(h0_texture, texelCoord, vec4(h0.xy, 0, 1));
    imageStore(wave_texture, texelCoord, wave);
}

float omega(float k) {
    return sqrt(g * k * tanh(min(k * D, 18)));
}

float omega_bar(float k) {
    return floor(omega(k) / omega_0) * omega_0;
}

float P_h(vec2 k) {
    float A = 0.00002;
    float k_length = length(k);
    if (k_length == 0) {
        return 0;
    }
    float k_length_2 = k_length * k_length;
    float k_length_4 = k_length_2 * k_length_2;
    float _L = V * V / g;
    float _L_2 = _L * _L;
    float k_dot_omega = dot(normalize(k), normalize(omega_hat));
    float k_dot_omega_2 = k_dot_omega * k_dot_omega;
    float l_2 = _L_2 * 0.001 * 0.001;
    return A * exp(-1 / (k_length_2 * _L_2)) / k_length_4 * k_dot_omega_2 * exp(-k_length_2 * l_2);
}

vec2 h_tilde_0(vec2 k) {
    return imageLoad(gaussianNoise, ivec2(gl_GlobalInvocationID.xy)).xy * sqrt(P_h(k) / 2.0);
}
