#pragma once

#include "minigl_types.h"
#include "object.h"
#include "pd_api.h"
#include "texture.h"

#define SCREEN_SIZE_X 400
#define SCREEN_SIZE_Y 240

// TODO: Make the library plaform agnostic
extern PlaydateAPI* pd;

// TODO: Create object
extern uint8_t c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
extern float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

// State objects
typedef struct {
    minigl_tex_t texture;
    minigl_tex_mode_t texture_mode;
    minigl_tex_t dither;
    minigl_dither_mode_t dither_mode;
} minigl_cfg_t;

void minigl_set_tex(minigl_tex_t t);

void minigl_set_dither(minigl_tex_t t);

void minigl_clear(int color, int depth);

void minigl_draw(minigl_obj_t obj);

void minigl_swap_frame(void);
