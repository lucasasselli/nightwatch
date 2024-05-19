#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "map.h"
#include "minigl.h"
#include "object.h"
#include "pd_api.h"
#include "utils.h"

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
minigl_tex_t tex_dither;

unsigned int update_cnt = 0;

void view_update(void) {
    // Update camera poistion
    vec3 camera_center;
    glm_vec3_add(gs.camera.pos, gs.camera.front, camera_center);
    glm_lookat(gs.camera.pos, camera_center, gs.camera.up, view);
}

void screen_update(void) {
    uint8_t *frame = pd->graphics->getFrame();
    int i = 0;
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            if (c_buff[i]) {
                clearpixel(frame, x, SCREEN_SIZE_Y - y - 1, 52);
            } else {
                setpixel(frame, x, SCREEN_SIZE_Y - y - 1, 52);
            }
            i++;
        }
    }
    pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

int64_t difftimespec_ns(const struct timespec after, const struct timespec before) {
    return ((int64_t)after.tv_sec - (int64_t)before.tv_sec) * (int64_t)1000000000 + ((int64_t)after.tv_nsec - (int64_t)before.tv_nsec);
}

static int update(void *userdata) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);
    if (pushed) {
        handle_keys(&gs, pushed);
        view_update();
    }

#ifdef DEBUG_PERF
    struct timespec start, stop;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
    }
#endif

    minigl_clear(0.0f, 1.0f);

    // Prepare the transformation matrix
    mat4 trans;
    glm_mat4_mul(proj, view, trans);

    // Draw map
    map_draw(trans, gs.camera);

    // Update the screen
    screen_update();

    minimap_draw(300, 0, gs.camera);

#ifdef DEBUG_PERF
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
    }
    debug("Update Time: %d", difftimespec_ns(stop, start));
#endif

    update_cnt++;

#ifdef DEBUG
    pd->system->drawFPS(0, 0);
#endif

    return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    if (event == kEventInit) {
        pd = _pd;

        pd->system->logToConsole("Begin setup...");

        //---------------------------------------------------------------------------
        // Device config
        //---------------------------------------------------------------------------

        pd->system->setUpdateCallback(update, pd);
        pd->display->setRefreshRate(30);

        //---------------------------------------------------------------------------
        // Game resources
        //---------------------------------------------------------------------------

        minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);

        //---------------------------------------------------------------------------
        // Game config
        //---------------------------------------------------------------------------

        // int seed = time(NULL);
        int seed = 0;
        debug("SEED: %d\n", seed);
        srand(seed);

        minigl_set_dither(tex_dither);

        // Setup map
        map_init();

        // Load fonts
        const char *err;
        font = pd->graphics->loadFont(fontpath, &err);

        if (font == NULL) {
            pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);
        }
        pd->graphics->setFont(font);

        // Initialize
        game_init(&gs);
        glm_perspective(glm_rad(60), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 0.1f, 100.0f, proj);
        view_update();  // Setup view matrix

        pd->system->logToConsole("Setup complete!");
    }

    return 0;
}
