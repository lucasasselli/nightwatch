#pragma once

#include "minigl/texture.h"

typedef enum {
    TEX_ID_DITHER,
    TEX_ID_ENEMY,
    TEX_ID_WETFLOOR,
    TEX_ID_NOTE,
    TEX_ID_WC_SINK,
    TEX_ID_SIGN_WC_MAN,
    TEX_ID_SIGN_WC_WOMEN,
    TEX_ID_SIGN_EMPLOYEEONLY,
    TEX_ID_PICTURE_0,
    TEX_ID_PICTURE_1,
    TEX_ID_PICTURE_2,
    TEX_ID_PICTURE_3,
    TEX_ID_PICTURE_4,
    TEX_ID_PICTURE_5,
    TEX_ID_PICTURE_6,
    TEX_ID_PICTURE_7,
    TEX_ID_PICTURE_8,
    TEX_ID_PICTURE_9,
    TEX_ID_PICTURE_10,
    TEX_ID_PICTURE_11,
    TEX_ID_PICTURE_12,
    TEX_ID_PICTURE_13,
    TEX_ID_PICTURE_14,
} tex_id_t;

#define TEX_ID_NUM 23

typedef enum {
    TEX_ID_VENUS
} tex_mdbb_id_t;

#define TEX_MDBB_ID_NUM 1
#define MDBB_SIZE 8
#define MDBB_STEP (360 / MDBB_SIZE)

int tex_load(tex_id_t id);

int tex_mdbb_load(tex_mdbb_id_t id);

int tex_load_all();

minigl_tex_t* tex_get(tex_id_t id);

minigl_tex_t* tex_mdbb_get_i(tex_mdbb_id_t id, int i);

minigl_tex_t* tex_mdbb_get_a(tex_mdbb_id_t id, float a);
