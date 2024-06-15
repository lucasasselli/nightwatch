#include "texture.h"

#include <assert.h>
#include <math.h>

#include "cglm/affine.h"

minigl_tex_t tex_array[TEX_NUM];
minigl_tex_t tex_mdbb_array[TEX_NUM][MDBB_SIZE];

int tex_init(void) {
    int result = 0;

    // TODO: Convert everything to bitmap!
    result |= minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_array[TEX_DITHER]);
    result |= minigl_tex_read_file("res/textures/monster_idle.tex", &tex_array[TEX_ENEMY]);
    result |= minigl_tex_read_file("res/textures/wetfloor.tex", &tex_array[TEX_WETFLOOR]);
    result |= minigl_tex_read_file("res/textures/note.tex", &tex_array[TEX_NOTE]);
    result |= minigl_tex_read_file("res/textures/fence_open.tex", &tex_array[TEX_FENCE_OPEN]);
    result |= minigl_tex_read_file("res/textures/fence_closed.tex", &tex_array[TEX_FENCE_CLOSED]);

    return result;
}

int tex_mdbb_load(tex_mdbb_id_t id) {
    char str_path[50];
    int result = 0;

    for (int i = 0; i < MDBB_SIZE; i++) {
        switch (id) {
            case TEX_VENUS:
                result |= sprintf(str_path, "res/sprites/venus/venus_%02d.tex", i);
        }

        sprintf(str_path, "res/sprites/venus/venus_%02d.tex", i);
        minigl_tex_read_file(str_path, &tex_mdbb_array[id][i]);
    }

    return result;
}

minigl_tex_t* tex_get(tex_id_t id) {
    assert(id < TEX_NUM);
    return &tex_array[id];
}

minigl_tex_t* tex_mdbb_get_i(tex_mdbb_id_t id, int i) {
    assert(id < TEX_NUM);
    return &tex_mdbb_array[id][i];
}

minigl_tex_t* tex_mdbb_get_a(tex_mdbb_id_t id, float a) {
    int i;

    // Convert to degrees
    glm_make_deg(&a);

    int a_int = ((int)fabsf(a)) % 360;

    if (a > 0) {
        i = a_int / 10;
    } else {
        i = (MDBB_SIZE - 1) - a_int / 10;
    }

    return tex_mdbb_get_i(id, i);
}
