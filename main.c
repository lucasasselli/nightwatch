#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "minigl.h"
#include "objread.h"
#include "pd_api.h"

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

// Constants
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

// API handle
PlaydateAPI *pd;

// Game state
game_state_t gs;
mat4 proj;
mat4 view;

// Resources
LCDFont *font = NULL;
minigl_obj_t obj_cube;
minigl_tex_t tex_mario;

#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorBlack : kColorWhite)

minigl_tex_t texture_read(char *path) {
    const char *error = NULL;
    LCDBitmap *bm = pd->graphics->loadBitmap(path, &error);
    if (error != NULL) {
        pd->system->error("%s", error);
    }

    minigl_tex_t t;

    int rowbytes;
    uint8_t *data;
    pd->graphics->getBitmapData(bm, &t.size_x, &t.size_y, &rowbytes, NULL, &data);

    t.ptr = (uint8_t **)malloc(t.size_y * sizeof(uint8_t *));

    for (int j = 0; j < t.size_y; j++) {
        t.ptr[j] = (uint8_t *)malloc(t.size_x * sizeof(uint8_t));
        for (int i = 0; i < t.size_x; i++) {
            t.ptr[j][i] = samplepixel(data, i, j, rowbytes);
        }
    }

    pd->graphics->freeBitmap(bm);

    return t;
}

static int update(void *userdata) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);
    if (pushed) {
        handle_keys(&gs, pushed);
        vec3 camera_center;
        glm_vec3_add(gs.camera.pos, gs.camera.front, camera_center);
        glm_lookat(gs.camera.pos, camera_center, gs.camera.up, view);
    }

    minigl_obj_t obj = obj_cube;

    // Copy the vertices
    obj.vcoord_ptr = (vec4 *)malloc(sizeof(vec4) * obj_cube.vcoord_size);
    memcpy(obj.vcoord_ptr, obj_cube.vcoord_ptr, sizeof(vec4) * obj_cube.vcoord_size);

    // Prepare the transformation matrix
    mat4 trans;
    glm_mat4_mul(proj, view, trans);

    // Shade the vertices!
    for (int i = 0; i < obj.vcoord_size; i++) {
        // Apply transformation
        glm_mat4_mulv(trans, obj.vcoord_ptr[i], obj.vcoord_ptr[i]);

        // Convert to carthesian coord.
        // FIXME: Keep w intact for frustrum based culling!
        glm_vec4_divs(obj.vcoord_ptr[i], obj.vcoord_ptr[i][3], obj.vcoord_ptr[i]);
    }

    minigl_clear(0, -1.0f);
    minigl_set_tex(tex_mario);
    minigl_draw(obj);
    minigl_swap_frame();

    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    if (event == kEventInit) {
        pd = _pd;

        pd->system->logToConsole("Begin setup...");

        // Device config
        pd->system->setUpdateCallback(update, pd);
        pd->display->setRefreshRate(30);

        // Load resources
        obj_cube = obj_file_read("res/models/cube.obj");
        tex_mario = texture_read("res/textures/mario.png");

        const char *err;
        font = pd->graphics->loadFont(fontpath, &err);

        if (font == NULL) {
            pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);
        }
        pd->graphics->setFont(font);

        // Initialize
        glm_perspective(glm_rad(60), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 0.1f, 10.0f, proj);
        game_init(&gs);

        pd->system->logToConsole("Setup complete!");
    }

    if (event == kEventKeyPressed) {
        // Unused
    }

    return 0;
}
