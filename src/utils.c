#include "utils.h"

int fake_printf(const char* format, ...) {
    (void)format;
    return 0;
}

int mini(int a, int b) {
    return (a < b) ? a : b;
}

int maxi(int a, int b) {
    return (a > b) ? a : b;
}

int clampi(int x, int min, int max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

float vec2_angle(vec2 a, vec2 b) {
    float dot = glm_vec2_dot(a, b);
    float det = a[0] * b[1] - a[1] * b[0];
    return atan2f(det, dot);
}

void vec2_to_ivec2(vec2 in, ivec2 out) {
    out[0] = roundf(in[0]);
    out[1] = roundf(in[1]);
}

void ivec2_to_vec2(ivec2 in, vec2 out) {
    out[0] = in[0];
    out[1] = in[1];
}

void ivec2_to_vec2_center(ivec2 in, vec2 out) {
    out[0] = in[0] + 0.5;
    out[1] = in[1] + 0.5;
}

void tile_dir(ivec2 tile, vec2 pos, vec2 out) {
    out[0] = (((float)tile[0]) + 0.5f) - pos[0];
    out[1] = (((float)tile[1]) + 0.5f) - pos[1];
    glm_vec2_normalize(out);
}
