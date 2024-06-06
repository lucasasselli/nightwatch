#pragma once

#define CAMERA_FOV 60
#define CAMERA_MIN_Z 0.1f
#define CAMERA_MAX_Z 40.0f

#define CAMERA_UP ((vec3){0.0f, 1.0f, 0.0f})
#define CAMERA_VEC3(x) ((vec3){x[0], 0.0f, x[1]})

#define INPUT_CAMERA_TSPEED1 0.1f
#define INPUT_CAMERA_TSPEED2 0.2f
#define INPUT_CAMERA_RSPEED 5.0f

#define MAP_SIZE 64
#define MAP_DRAW_SIZE 20
