#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "map.h"
#include "minigl.h"
#include "object.h"
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
minigl_obj_t obj_my;
minigl_tex_t tex_dither;

unsigned int update_cnt = 0;

void view_update(void) {
    // Update camera poistion
    vec3 camera_center;
    glm_vec3_add(gs.camera.pos, gs.camera.front, camera_center);
    glm_lookat(gs.camera.pos, camera_center, gs.camera.up, view);
}

static int update(void *userdata) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);
    if (pushed) {
        handle_keys(&gs, pushed);
        view_update();
    }

    minigl_clear(0.0f, 1.0f);

    // Prepare the transformation matrix
    mat4 trans;
    glm_mat4_mul(proj, view, trans);

    // Draw map
    map_draw(trans);

    minigl_swap_frame();

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

        //---------------------------------------------------------------------------
        // Game config
        //---------------------------------------------------------------------------

        // int seed = time(NULL);
        int seed = 0;
        debug("SEED: %d\n", seed);
        srand(seed);

        // Load resources

        // Load model
        minigl_obj_read_file("res/models/cube.obj", &obj_my);

        minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);

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

    if (event == kEventKeyPressed) {
        // Unused
    }

    return 0;
}
