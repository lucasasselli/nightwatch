#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "map_generator.h"
#include "map_renderer.h"
#include "minigl.h"
#include "pd_api.h"
#include "pd_system.h"
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
mat4 trans;

// Resources
LCDFont *font = NULL;
minigl_tex_t tex_dither;

unsigned int update_cnt = 0;

void view_update(void) {
    // Update camera poistion
    vec3 camera_center;
    mat4 view;
    glm_vec3_add(gs.camera.pos, gs.camera.front, camera_center);
    glm_lookat(gs.camera.pos, camera_center, gs.camera.up, view);
    glm_mat4_mul(proj, view, trans);
}

// FIXME: Write directly on the screen buffer?
void screen_update(void) {
    uint8_t *frame = pd->graphics->getFrame();
    int i = 0;
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            if (c_buff[i]) {
                clearpixel(frame, x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            } else {
                setpixel(frame, x, SCREEN_SIZE_Y - y - 1, LCD_ROWSIZE);
            }
            i++;
        }
    }
    pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

void minigl_perf_print(void) {
    minigl_perf_data_t perf_data = minigl_perf_get();
#ifdef DEBUG_PERF
    debug("Clip count: %d", perf_data.clip);
    debug("Cull count: %d", perf_data.cull);
    debug("Poly count: %d", perf_data.poly);
    debug("Frag count: %d", perf_data.frag);
#endif
}

static int update(void *userdata) {
    // Handle keys
    PDButtons pushed;
    pd->system->getButtonState(&pushed, NULL, NULL);
    if (pushed) {
        handle_keys(&gs, pushed);
        view_update();
    }

    meas_time_start(0);

    meas_time_start(1);
    minigl_clear(0.0f, 1.0f);
    meas_time_stop(1);

    // Draw map
    meas_time_start(2);
    map_draw(gs.map, trans, gs.camera);
    meas_time_stop(2);

    // Update the screen
    meas_time_start(3);
    screen_update();
    meas_time_stop(3);

    meas_time_start(4);
    minimap_draw(300, 0, gs.camera);
    meas_time_stop(4);

    meas_time_stop(0);

#ifdef DEBUG_PERF
    if (update_cnt % 100 == 0) {
        meas_time_print(1, "Buffer flush ");
        meas_time_print(2, "Map draw     ");
        meas_time_print(3, "Screen update");
        meas_time_print(4, "Minimap draw ");
        meas_time_print(0, "Full         ");
        minigl_perf_print();
    }
    minigl_perf_clear();
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

        debug("Begin setup...");

        //---------------------------------------------------------------------------
        // Device config
        //---------------------------------------------------------------------------

        pd->system->setUpdateCallback(update, pd);
        pd->display->setRefreshRate(30);

        //---------------------------------------------------------------------------
        // Game resources
        //---------------------------------------------------------------------------

        minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);

        // Load fonts
        const char *err;
        font = pd->graphics->loadFont(fontpath, &err);

        if (font == NULL) {
            pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);
        }
        pd->graphics->setFont(font);

        //---------------------------------------------------------------------------
        // Game config
        //---------------------------------------------------------------------------

        // int seed = time(NULL);
        int seed = 0;
        debug("SEED: %d", seed);
        srand(seed);

        minigl_set_dither(tex_dither);

        mapgen_gen(gs.map);
        mapgen_grid_print(gs.map);

        // Setup map
        map_init();

        // Setup minimap
        minimap_init();
        minimap_gen(gs.map);

        // Initialize game
        game_init(&gs);

        // Pick a random starting position in the map
        map_tile_t spawn_tile;
        do {
            ivec2 spawn_tile_pos;
            spawn_tile_pos[0] = rand() % MAP_SIZE;
            spawn_tile_pos[1] = rand() % MAP_SIZE;
            spawn_tile = map_get_tile(gs.map, spawn_tile_pos);
            pos_tile_to_world(spawn_tile_pos, gs.camera.pos);
        } while (map_tile_collide(spawn_tile));

        glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 1.0f, 20.0f, proj);
        view_update();  // Setup view matrix

        debug("Setup complete!");
    }

    return 0;
}
