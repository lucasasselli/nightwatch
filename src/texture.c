#include "texture.h"

#include <assert.h>
#include <cglm/cglm.h>
#include <math.h>

#include "utils.h"

minigl_tex_t tex_array[TEX_ID_NUM];
minigl_tex_t tex_mdbb_array[TEX_MDBB_ID_NUM][MDBB_SIZE];

#define TEXID_LOAD_CASE(path, id, opts) \
    case id:                            \
        return tex_file_read(path, id, opts)

static int tex_file_read(const char* path, tex_id_t id, minigl_tex_read_opts_t opts) {
    debug("Reading %s ...", path);
    return minigl_tex_read_file(path, &tex_array[id], opts);
}

int tex_load(tex_id_t id) {
    switch (id) {
        case TEX_ID_DITHER:
            return tex_file_read("res/dither/bayer16tile2.tex", TEX_ID_DITHER, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_ENEMY:
            return tex_file_read("res/textures/monster_idle.tex", TEX_ID_ENEMY, MINIGL_TEX_READ_OPTS_NONE);
        case TEX_ID_WETFLOOR:
            return tex_file_read("res/textures/wetfloor.tex", TEX_ID_WETFLOOR, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_NOTE:
            return tex_file_read("res/textures/note.tex", TEX_ID_NOTE, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_WC_SINK:
            return tex_file_read("res/textures/sink.tex", TEX_ID_WC_SINK, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_SIGN_WC_MAN:
            return tex_file_read("res/textures/sign_wc_man.tex", TEX_ID_SIGN_WC_MAN, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_SIGN_WC_WOMEN:
            return tex_file_read("res/textures/sign_wc_women.tex", TEX_ID_SIGN_WC_WOMEN, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_SIGN_EMPLOYEEONLY:
            return tex_file_read("res/textures/sign_employeeonly.tex", TEX_ID_SIGN_EMPLOYEEONLY, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_0:
            return tex_file_read("res/textures/picture_owl.tex", TEX_ID_PICTURE_0, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_1:
            return tex_file_read("res/textures/picture_key.tex", TEX_ID_PICTURE_1, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_2:
            return tex_file_read("res/textures/picture_sword.tex", TEX_ID_PICTURE_2, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_3:
            return tex_file_read("res/textures/picture_telescope.tex", TEX_ID_PICTURE_3, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_4:
            return tex_file_read("res/textures/picture_horse.tex", TEX_ID_PICTURE_4, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_PICTURE_5:
            return tex_file_read("res/textures/picture_man.tex", TEX_ID_PICTURE_5, (minigl_tex_read_opts_t){.force_g8 = true});
        case TEX_ID_WHITEBOARD:
            return tex_file_read("res/textures/whiteboard.tex", TEX_ID_WHITEBOARD, (minigl_tex_read_opts_t){.force_g8 = true});
    }
}

int tex_mdbb_load(tex_mdbb_id_t id) {
    char path[50];
    int result = 0;

    for (int i = 0; i < MDBB_SIZE; i++) {
        switch (id) {
            case TEX_ID_VENUS:
                result |= sprintf(path, "res/sprites/venus/venus_%02d.tex", i);
        }

        debug("Reading %s ...", path);
        sprintf(path, "res/sprites/venus/venus_%02d.tex", i);
        minigl_tex_read_file(path, &tex_mdbb_array[id][i], MINIGL_TEX_READ_OPTS_NONE);
    }

    return result;
}

minigl_tex_t* tex_get(tex_id_t id) {
    assert(id < TEX_ID_NUM);
    return &tex_array[id];
}

minigl_tex_t* tex_mdbb_get_i(tex_mdbb_id_t id, int i) {
    assert(id < TEX_MDBB_ID_NUM);
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
