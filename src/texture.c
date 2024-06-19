#include "texture.h"

#include <assert.h>
#include <cglm/cglm.h>
#include <math.h>

#include "utils.h"

minigl_tex_t tex_array[TEX_ID_NUM];
minigl_tex_t tex_mdbb_array[TEX_MDBB_ID_NUM][MDBB_SIZE];

#define TEXID_LOAD_CASE(path, id) \
    case id:                      \
        return tex_file_read(path, id)

static int tex_file_read(const char* path, tex_id_t id) {
    debug("Reading %s ...", path);
    return minigl_tex_read_file(path, &tex_array[id]);
}

int tex_load(tex_id_t id) {
    switch (id) {
        TEXID_LOAD_CASE("res/dither/bayer16tile2.tex", TEX_ID_DITHER);
        TEXID_LOAD_CASE("res/textures/monster_idle.tex", TEX_ID_ENEMY);
        TEXID_LOAD_CASE("res/textures/wetfloor.tex", TEX_ID_WETFLOOR);
        TEXID_LOAD_CASE("res/textures/note.tex", TEX_ID_NOTE);
        TEXID_LOAD_CASE("res/textures/wood.tex", TEX_ID_WOOD);
        TEXID_LOAD_CASE("res/textures/sink.tex", TEX_ID_WC_SINK);
        TEXID_LOAD_CASE("res/textures/museum_history.tex", TEX_ID_SIGN_MUSEUM_HISTORY);
        TEXID_LOAD_CASE("res/textures/sign_wc_man.tex", TEX_ID_SIGN_WC_MAN);
        TEXID_LOAD_CASE("res/textures/sign_wc_women.tex", TEX_ID_SIGN_WC_WOMEN);
        TEXID_LOAD_CASE("res/textures/picture_owl.tex", TEX_ID_PICTURE_0);
        TEXID_LOAD_CASE("res/textures/picture_key.tex", TEX_ID_PICTURE_1);
        TEXID_LOAD_CASE("res/textures/picture_sword.tex", TEX_ID_PICTURE_2);
        TEXID_LOAD_CASE("res/textures/picture_telescope.tex", TEX_ID_PICTURE_3);
        TEXID_LOAD_CASE("res/textures/picture_horse.tex", TEX_ID_PICTURE_4);
        TEXID_LOAD_CASE("res/textures/picture_man.tex", TEX_ID_PICTURE_5);
        TEXID_LOAD_CASE("res/textures/single_digit_0.tex", TEX_ID_COLUMN_DIG_0);
        TEXID_LOAD_CASE("res/textures/single_digit_1.tex", TEX_ID_COLUMN_DIG_1);
        TEXID_LOAD_CASE("res/textures/single_digit_3.tex", TEX_ID_COLUMN_DIG_3);
        TEXID_LOAD_CASE("res/textures/single_digit_4.tex", TEX_ID_COLUMN_DIG_4);
        TEXID_LOAD_CASE("res/textures/single_digit_6.tex", TEX_ID_COLUMN_DIG_6);
        TEXID_LOAD_CASE("res/textures/single_digit_7.tex", TEX_ID_COLUMN_DIG_7);
        TEXID_LOAD_CASE("res/textures/column_cypher.tex", TEX_ID_COLUMN_CYPHER);
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
        minigl_tex_read_file(path, &tex_mdbb_array[id][i]);
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
