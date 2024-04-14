#include <stdio.h>
#include <stdlib.h>

#include "minigl.h"
#include "objread.h"
#include "pd_api.h"

static int update(void *userdata);
void setup(PlaydateAPI *pd);
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont *font = NULL;

PlaydateAPI *pd;

int setup_done = 0;
float r = 0;

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t arg) {
    (void)arg;  // arg is currently only used for event = kEventKeyPressed

    if (!setup_done) {
        pd->system->logToConsole("Begin setup...");
        setup(pd);
        pd->system->logToConsole("Setup complete!");
        setup_done = 1;
    }

    if (event == kEventInit) {
        const char *err;
        font = pd->graphics->loadFont(fontpath, &err);

        if (font == NULL) pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

        // Note: If you set an update callback in the kEventInit handler, the
        // system assumes the game is pure C and doesn't run any Lua code in the
        // game
        pd->system->setUpdateCallback(update, pd);
    }

    return 0;
}

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

obj_data_t cube;

void setup(PlaydateAPI *_pd) {
    pd = _pd;
    cube = obj_file_read("res/models/cube.obj");
}

static int update(void *userdata) {
    PlaydateAPI *pd = userdata;

    pd->graphics->clear(kColorWhite);
    // pd->graphics->setFont(font);
    // pd->graphics->drawText("Hello World!", strlen("Hello World!"),
    // kASCIIEncoding, x, y);

    obj_data_t obj = cube;

    // Copy the vertices
    obj.vptr = (vec4_t *)malloc(sizeof(vec4_t) * cube.vptr_size);
    memcpy(obj.vptr, cube.vptr, sizeof(vec4_t) * cube.vptr_size);

    for (int i = 0; i < obj.vptr_size; i++) {
        vertex_scale(&obj.vptr[i], 1);
        vertex_move(&obj.vptr[i], 0, 0, -7, r);
        minigl_perspective(&obj.vptr[i], M_PI / 3, 0.6f, 1, 20);
    }

    minigl_clear(0, -1);
    minigl_draw(obj.vptr, obj.fptr, obj.fptr_size);
    minigl_swap_frame();

    r += M_PI / 100;

    pd->system->drawFPS(0, 0);

    return 1;
}
