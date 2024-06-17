#pragma once

#include <cglm/cglm.h>

#define CAMERA_FOV 60
#define CAMERA_MIN_Z 0.1f
#define CAMERA_MAX_Z 10.0f

#define CAMERA_UP ((vec3){0.0f, 1.0f, 0.0f})
#define CAMERA_VEC3(x) ((vec3){x[0], 0.0f, x[1]})

typedef struct {
    vec2 pos;
    float bob;
    vec2 front;
    float yaw;
    mat4 proj;
    mat4 trans;
} camera_t;

void camera_init(camera_t* camera);

void camera_update_trans(camera_t* camera);
