#include "utils.h"

int fake_printf(const char* format, ...) {
    (void)format;
    return 0;
}

int rand_range(int min, int max) {
    assert(max > min);
    return min + (rand() % (max - min));
}

int mini(int a, int b) {
    return (a < b) ? a : b;
}

int maxi(int a, int b) {
    return (a > b) ? a : b;
}

float vec2_angle(vec2 a, vec2 b) {
    float dot = glm_vec2_dot(a, b);
    float det = a[0] * b[1] - a[1] * b[0];
    return atan2f(det, dot);
}

void vec2_to_ivec2(vec2 in, ivec2 out) {
    out[0] = in[0];
    out[1] = in[1];
}

void ivec2_to_vec2(ivec2 in, vec2 out) {
    out[0] = in[0];
    out[1] = in[1];
}

void ivec2_to_vec2_center(ivec2 in, vec2 out) {
    out[0] = in[0] + 0.5;
    out[1] = in[1] + 0.5;
}

void ivec2_to_vec2_center(ivec2, vec2 out);

void mat4_billboard(camera_t camera, mat4 trans) {
    vec2 poly_dir2;
    poly_dir2[0] = 0.0f;
    poly_dir2[1] = -1.0f;
    // glm_vec2_normalize(poly_dir2);

    float a = vec2_angle(poly_dir2, camera.front);
    glm_rotate_at(trans, GLM_VEC3_ZERO, -a, (vec3){0.0f, 1.0f, 0.0f});
}

void tile_dir(ivec2 tile, vec2 pos, vec2 out) {
    out[0] = (((float)tile[0]) + 0.5f) - pos[0];
    out[1] = (((float)tile[1]) + 0.5f) - pos[1];
    glm_vec2_normalize(out);
}
