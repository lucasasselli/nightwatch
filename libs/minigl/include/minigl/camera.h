#pragma once

#include <cglm/cglm.h>

typedef struct {
    vec3 pos;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
} minigl_camera_t;
