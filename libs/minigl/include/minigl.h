#pragma once

#include <stdio.h>

#include "minigl_types.h"
#include "pd_api.h"

#define SCREEN_SIZE_X 400
#define SCREEN_SIZE_Y 240

// TODO: Make the library plaform agnostic
extern PlaydateAPI* pd;

// TODO: Create object
extern char c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
extern float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

// State objects
typedef struct {
    minigl_texture_t texture;
    minigl_texture_mode_t texture_mode;
} minigl_cfg_t;

void vertex_scale(vec4* v, float k);

void vertex_move(vec4* v, float x, float y, float z, float r);

void minigl_perspective(vec4* v, float camera_fov, float camera_ratio, float clip_near, float clip_far);

void minigl_set_texture(minigl_texture_t t);

void minigl_clear(int color, int depth);

void minigl_draw(minigl_obj_t obj);

void minigl_swap_frame(void);
