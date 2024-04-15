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

minigl_obj_t cube;

LCDBitmap *bm;

#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorBlack : kColorWhite)

void setup(PlaydateAPI *_pd) {
    pd = _pd;
    cube = obj_file_read("res/models/cube.obj");

    const char *error = NULL;
    LCDBitmap *bm = pd->graphics->loadBitmap("res/textures/mario.png", &error);
    if (error != NULL) {
        pd->system->logToConsole("%s", error);
    }

    int width;
    int height;
    int rowbytes;
    uint8_t *data;
    pd->graphics->getBitmapData(bm, &width, &height, &rowbytes, NULL, &data);

    uint8_t **tex = (uint8_t **)malloc(height * sizeof(uint8_t *));

    for (int j = 0; j < height; j++) {
        tex[j] = (uint8_t *)malloc(width * sizeof(uint8_t));
        for (int i = 0; i < width; i++) {
            tex[j][i] = samplepixel(data, i, j, rowbytes);
        }
    }

    minigl_set_texture(tex, width, height);
}

static int update(void *userdata) {
    PlaydateAPI *pd = userdata;

    pd->graphics->clear(kColorWhite);
    // pd->graphics->setFont(font);
    // pd->graphics->drawText("Hello World!", strlen("Hello World!"),
    // kASCIIEncoding, x, y);

    minigl_obj_t obj = cube;

    // Copy the vertices
    obj.vcoord_ptr = (vec4_t *)malloc(sizeof(vec4_t) * cube.vcoord_size);
    memcpy(obj.vcoord_ptr, cube.vcoord_ptr, sizeof(vec4_t) * cube.vcoord_size);

    for (int i = 0; i < obj.vcoord_size; i++) {
        vertex_scale(&obj.vcoord_ptr[i], 1);
        vertex_move(&obj.vcoord_ptr[i], 0, 0, -3, r);
        minigl_perspective(&obj.vcoord_ptr[i], M_PI / 3, 0.6f, 1, 20);
    }

    minigl_clear(0, -1);
    minigl_draw(obj);
    minigl_swap_frame();

    r += M_PI / 100;

    pd->system->drawFPS(0, 0);

    return 1;
}
