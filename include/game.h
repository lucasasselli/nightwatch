#pragma once

#include "minigl_types.h"
#include "pd_api.h"

#define INPUT_MOUSE_SENSITIVITY 0.4f
#define INPUT_CAMERA_SPEED 0.05f

extern PlaydateAPI* pd;

typedef struct {
    vec2_t pos;
    float rot;
} camera_t;
