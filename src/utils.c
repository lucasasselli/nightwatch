#include "utils.h"

#include <time.h>

struct timespec meas_time_start_data[5];
struct timespec meas_time_stop_data[5];

int fake_printf(const char* format, ...) {
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

bool ivec2_eq(ivec2 a, ivec2 b) {
    return a[0] == b[0] && a[1] == b[1];
}

void mat4_billboard(minigl_camera_t camera, mat4 trans) {
    vec2 camera_dir2;
    camera_dir2[0] = camera.front[0];
    camera_dir2[1] = camera.front[2];
    // glm_vec2_normalize(camera_dir2);

    vec2 poly_dir2;
    poly_dir2[0] = 0.0f;
    poly_dir2[1] = -1.0f;
    // glm_vec2_normalize(poly_dir2);

    float a = vec2_angle(poly_dir2, camera_dir2);
    glm_rotate_at(trans, GLM_VEC3_ZERO, -a, (vec3){0.0f, 1.0f, 0.0f});
}

int64_t difftimespec_ns(const struct timespec after, const struct timespec before) {
    return ((int64_t)after.tv_sec - (int64_t)before.tv_sec) * (int64_t)1000000000 + ((int64_t)after.tv_nsec - (int64_t)before.tv_nsec);
}

void meas_time_start(int id) {
#ifdef DEBUG_PERF
    clock_gettime(CLOCK_REALTIME, &meas_time_start_data[id]);
#endif
}

void meas_time_stop(int id) {
#ifdef DEBUG_PERF
    clock_gettime(CLOCK_REALTIME, &meas_time_stop_data[id]);
#endif
}

void meas_time_print(int id, const char* msg) {
#ifdef DEBUG_PERF
    pd->system->logToConsole("%s: %ld", msg, difftimespec_ns(meas_time_stop_data[id], meas_time_start_data[id]));
#endif
}
