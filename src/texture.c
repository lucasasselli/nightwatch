#include "texture.h"

#include <assert.h>
#include <cglm/cglm.h>
#include <math.h>

#include "utils.h"

minigl_tex_t tex_array[TEX_NUM];
minigl_tex_t tex_mdbb_array[TEX_NUM][MDBB_SIZE];

static int tex_file_read(const char* path, tex_id_t id) {
    debug("Reading %s ...", path);
    return minigl_tex_read_file(path, &tex_array[id]);
}

int tex_init(void) {
    int result = 0;

    // TODO: Convert everything to bitmap!
    result |= tex_file_read("res/dither/bayer16tile2.tex", TEX_DITHER);
    result |= tex_file_read("res/textures/base.tex", TEX_BASE);
    result |= tex_file_read("res/textures/monster_idle.tex", TEX_ENEMY);
    result |= tex_file_read("res/textures/wetfloor.tex", TEX_WETFLOOR);
    result |= tex_file_read("res/textures/note.tex", TEX_NOTE);
    result |= tex_file_read("res/textures/fence_open.tex", TEX_FENCE_OPEN);
    result |= tex_file_read("res/textures/fence_closed.tex", TEX_FENCE_CLOSED);
    result |= tex_file_read("res/textures/wood.tex", TEX_WOOD);
    result |= tex_file_read("res/textures/sink.tex", TEX_WC_SINK);
    result |= tex_file_read("res/textures/sign_wc_man.tex", TEX_SIGN_WC_MAN);
    result |= tex_file_read("res/textures/sign_wc_women.tex", TEX_SIGN_WC_WOMEN);
    result |= tex_file_read("res/textures/single_digit_0.tex", TEX_COLUMN_DIG_0);
    result |= tex_file_read("res/textures/single_digit_1.tex", TEX_COLUMN_DIG_1);
    result |= tex_file_read("res/textures/single_digit_3.tex", TEX_COLUMN_DIG_3);
    result |= tex_file_read("res/textures/single_digit_4.tex", TEX_COLUMN_DIG_4);
    result |= tex_file_read("res/textures/single_digit_6.tex", TEX_COLUMN_DIG_6);
    result |= tex_file_read("res/textures/single_digit_7.tex", TEX_COLUMN_DIG_7);
    result |= tex_file_read("res/textures/level1_code_cypher.tex", TEX_LVL1_CYPHER);

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
