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
map_t map;
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

    // Draw map
    map_draw(map, trans, gs.camera);

    // Update the screen
    screen_update();

    minimap_draw(300, 0, gs.camera);

#ifdef DEBUG_PERF
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
    }
    if (update_cnt % 100 == 0) {
        debug("Update Time: %d", difftimespec_ns(stop, start));
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

        mapgen_init(&map);
        mapgen_gen(&map);

        // Setup map
        map_init();

        // Setup minimap
        minimap_init();
        minimap_gen(map);

        // Initialize game
        game_init(&gs);

        // Pick a random starting position in the map
        bool good = false;
        do {
            int x = rand() % MAP_SIZE;
            int y = rand() % MAP_SIZE;
            map_tile_t tile = map.grid[y][x];

            // Check if position is good
            if (tile.item_cnt == 1) {
                if (tile.items[0].type == TILE_FLOOR) {
                    good = true;
                }
            }

            gs.camera.pos[0] = x * MAP_TILE_SIZE;
            gs.camera.pos[2] = y * MAP_TILE_SIZE;
        } while (!good);

        glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 0.1f, 30.0f, proj);
        view_update();  // Setup view matrix

        debug("Setup complete!");
    }

    return 0;
}
