#pragma once

#include "minigl/texture.h"

typedef enum {
    TEX_DITHER,
    TEX_BASE,
    TEX_ENEMY,
    TEX_WETFLOOR,
    TEX_NOTE,
    TEX_FENCE_OPEN,
    TEX_FENCE_CLOSED,
    TEX_WOOD,
    TEX_WC_SINK,
    TEX_SIGN_WC_MAN,
    TEX_SIGN_WC_WOMEN,
    TEX_COLUMN_DIG_0,
    TEX_COLUMN_DIG_1,
    TEX_COLUMN_DIG_3,
    TEX_COLUMN_DIG_4,
    TEX_COLUMN_DIG_6,
    TEX_COLUMN_DIG_7,
    TEX_LVL1_CYPHER
} tex_id_t;

#define TEX_NUM 18

typedef enum {
    TEX_VENUS
} tex_mdbb_id_t;

#define TEX_MDBB_NUM 1
#define MDBB_SIZE 36

int tex_init(void);

int tex_mdbb_load(tex_mdbb_id_t id);

minigl_tex_t* tex_get(tex_id_t id);

minigl_tex_t* tex_mdbb_get_i(tex_mdbb_id_t id, int i);

minigl_tex_t* tex_mdbb_get_a(tex_mdbb_id_t id, float a);
