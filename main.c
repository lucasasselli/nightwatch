#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "minigl.h"
#include "objread.h"
#include "pd_api.h"

static int update(void *userdata);
void setup(PlaydateAPI *pd);
const char *fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont *font = NULL;

PlaydateAPI *pd;

// Game state
camera_t camera;

float r = 0;
#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

minigl_obj_t cube;

#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorBlack : kColorWhite)

minigl_texture_t texture_read(char *path) {
    const char *error = NULL;
    LCDBitmap *bm = pd->graphics->loadBitmap("res/textures/mario.png", &error);
    if (error != NULL) {
        pd->system->error("%s", error);
    }

    minigl_texture_t t;

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

    // TODO: Free bitmap

    return t;
}

static int update(void *userdata) {
    PlaydateAPI *pd = userdata;

    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);

    if (pushed) {
        pd->system->logToConsole("%x", pushed);
        if (pushed & kButtonA) {
        }
        if (pushed & kButtonUp) {
            camera.pos.coord.y += INPUT_CAMERA_SPEED;
        }
        if (pushed & kButtonDown) {
            camera.pos.coord.y -= INPUT_CAMERA_SPEED;
        }
        if (pushed & kButtonRight) {
            camera.rot -= INPUT_CAMERA_SPEED;
        }
        if (pushed & kButtonLeft) {
            camera.rot += INPUT_CAMERA_SPEED;
        }
    }

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
        // FIXME: This is wrong!
        vertex_move(&obj.vcoord_ptr[i], 0, 0, camera.pos.coord.y, camera.rot);
        minigl_perspective(&obj.vcoord_ptr[i], M_PI / 3, 0.6f, 1, 20);
    }

    minigl_clear(0, -1);
    minigl_draw(obj);
    minigl_swap_frame();

    r += M_PI / 100;

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
        cube = obj_file_read("res/models/cube.obj");

        minigl_texture_t mario = texture_read("res/textures/mario.png");

        minigl_set_texture(mario);  // TODO: Move
        pd->system->logToConsole("Setup complete!");
    }

    if (event == kEventKeyPressed) {
        // TODO:
    }

    return 0;
}
