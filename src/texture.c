#include "texture.h"

#include <assert.h>
#include <cglm/cglm.h>

#include "minigl/system.h"
#include "utils.h"

minigl_tex_t tex_array[TEX_ID_NUM];
minigl_tex_t tex_mdbb_array[TEX_MDBB_ID_NUM][MDBB_SIZE];

#define TEXID_LOAD_CASE(path, id, opts) \
    case id:                            \
        return tex_file_read(path, id, opts)

static int tex_file_read(const char *path, tex_id_t id) {
    debug("Reading %s ...", path);
    return minigl_tex_read_tex(path, &tex_array[id]);
}

int tex_load(tex_id_t id) {
    switch (id) {
        case TEX_ID_DITHER:
            return tex_file_read("res/textures/dither/bayer16tile2.tex", TEX_ID_DITHER);
        case TEX_ID_ENEMY:
            return tex_file_read("res/textures/monster_idle.tex", TEX_ID_ENEMY);
        case TEX_ID_WETFLOOR:
            return tex_file_read("res/textures/wetfloor.tex", TEX_ID_WETFLOOR);
        case TEX_ID_NOTE:
            return tex_file_read("res/textures/note.tex", TEX_ID_NOTE);
        case TEX_ID_WC_SINK:
            return tex_file_read("res/textures/sink.tex", TEX_ID_WC_SINK);  // TODO: Replace with material
        case TEX_ID_SIGN_WC_MAN:
            return tex_file_read("res/textures/sign_wc_man.tex", TEX_ID_SIGN_WC_MAN);
        case TEX_ID_SIGN_WC_WOMEN:
            return tex_file_read("res/textures/sign_wc_women.tex", TEX_ID_SIGN_WC_WOMEN);
        case TEX_ID_SIGN_EMPLOYEEONLY:
            return tex_file_read("res/textures/sign_employeeonly.tex", TEX_ID_SIGN_EMPLOYEEONLY);
        case TEX_ID_PICTURE_0:
            return tex_file_read("res/textures/picture_0.tex", TEX_ID_PICTURE_0);
        case TEX_ID_PICTURE_1:
            return tex_file_read("res/textures/picture_1.tex", TEX_ID_PICTURE_1);
        case TEX_ID_PICTURE_2:
            return tex_file_read("res/textures/picture_2.tex", TEX_ID_PICTURE_2);
        case TEX_ID_PICTURE_3:
            return tex_file_read("res/textures/picture_3.tex", TEX_ID_PICTURE_3);
        case TEX_ID_PICTURE_4:
            return tex_file_read("res/textures/picture_4.tex", TEX_ID_PICTURE_4);
        case TEX_ID_PICTURE_5:
            return tex_file_read("res/textures/picture_5.tex", TEX_ID_PICTURE_5);
        case TEX_ID_PICTURE_6:
            return tex_file_read("res/textures/picture_6.tex", TEX_ID_PICTURE_6);
        case TEX_ID_PICTURE_7:
            return tex_file_read("res/textures/picture_7.tex", TEX_ID_PICTURE_7);
        case TEX_ID_PICTURE_8:
            return tex_file_read("res/textures/picture_8.tex", TEX_ID_PICTURE_8);
        case TEX_ID_PICTURE_9:
            return tex_file_read("res/textures/picture_9.tex", TEX_ID_PICTURE_9);
        case TEX_ID_PICTURE_10:
            return tex_file_read("res/textures/picture_10.tex", TEX_ID_PICTURE_10);
        case TEX_ID_PICTURE_11:
            return tex_file_read("res/textures/picture_11.tex", TEX_ID_PICTURE_11);
        case TEX_ID_PICTURE_12:
            return tex_file_read("res/textures/picture_12.tex", TEX_ID_PICTURE_12);
        case TEX_ID_PICTURE_13:
            return tex_file_read("res/textures/picture_13.tex", TEX_ID_PICTURE_13);
        case TEX_ID_PICTURE_14:
            return tex_file_read("res/textures/picture_14.tex", TEX_ID_PICTURE_14);
    }

    return 0;
}

int tex_mdbb_load(tex_mdbb_id_t id) {
    char path[50];
    int result = 0;

    for (int i = 0; i < MDBB_SIZE; i++) {
        switch (id) {
            case TEX_ID_VENUS:
                sprintf(path, "res/textures/sprites/venus/%02d.tex", i);
        }
        debug("Reading %s ...", path);
        result |= minigl_tex_read_tex(path, &tex_mdbb_array[id][i]);
    }

    return result;
}

int tex_load_all(void) {
    for (int i = 0; i < TEX_ID_NUM; i++) {
        if (tex_load(i)) return 1;
    }

    for (int i = 0; i < TEX_MDBB_ID_NUM; i++) {
        if (tex_mdbb_load(i)) return 1;
    }

    return 0;
}

minigl_tex_t *tex_get(tex_id_t id) {
    assert(id < TEX_ID_NUM);
    return &tex_array[id];
}

minigl_tex_t *tex_mdbb_get_i(tex_mdbb_id_t id, int i) {
    assert(id < TEX_MDBB_ID_NUM);
    return &tex_mdbb_array[id][i];
}

minigl_tex_t *tex_mdbb_get_a(tex_mdbb_id_t id, float a) {
    // Convert to degrees
    glm_make_deg(&a);

    int a_int = (MDBB_SIZE + (int)(a / MDBB_STEP)) % MDBB_SIZE;

    return tex_mdbb_get_i(id, a_int);
}
