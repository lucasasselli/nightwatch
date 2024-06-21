#include <cglm/cglm.h>
#include <stdlib.h>

#include "game.h"
#include "game_state.h"
#include "minigl/minigl.h"
#include "pd_api.h"
#include "pd_system.h"
#include "sound.h"
#include "utils.h"
#include "view.h"

#define LOAD_STEP_LOCAL_CNT 3
#define LOAD_STEP_CNT LOAD_STEP_LOCAL_CNT + TEX_ID_NUM + TEX_MDBB_ID_NUM

// API handle
PlaydateAPI *pd;

game_state_t gs;

//---------------------------------------------------------------------------
// Global vars
//---------------------------------------------------------------------------

int load_step = 0;
float update_et_last = 0.0f;
int update_cnt = 0;

player_state_t player_state_old;
float player_state_time = 0;
bool prompt_shown = false;
bool screen_clear_req = false;

void print_perf(void) {
#ifdef MINIGL_DEBUG_PERF
    minigl_perf_data_t perf_data = minigl_perf_get();
    debug("Clip count: %d", perf_data.clip);
    debug("Cull count: %d", perf_data.cull);
    debug("Poly count: %d", perf_data.poly);
    debug("Frag count: %d", perf_data.frag);
#endif
}

static int lua_reset(lua_State *L) {
    (void)L;  // Unused
    game_reset();
    return 0;
}

static int lua_load(lua_State *L) {
    (void)L;  // Unused

    if (load_step < TEX_ID_NUM) {
        // Load textures
        if (tex_load(load_step)) {
            pd->system->logToConsole("%s:%i: Can't load texture id=%d!", __FILE__, __LINE__, load_step);
        }
    } else if (load_step < (TEX_MDBB_ID_NUM + TEX_ID_NUM)) {
        tex_mdbb_load(load_step - TEX_ID_NUM);
    } else if (load_step < (LOAD_STEP_LOCAL_CNT + TEX_ID_NUM + TEX_MDBB_ID_NUM)) {
        switch (load_step - TEX_ID_NUM - TEX_MDBB_ID_NUM) {
            case 0:
                sound_init();  // Load sound
                view_init();   // Load view
                break;

            case 1:
                // Load objects
                obj_init();
                break;

            case 2:
                // Initialize game
                game_reset();
                break;
        }
    } else {
        debug("Loading done!");
    }
    load_step++;

    pd->lua->pushInt(load_step);
    pd->lua->pushInt(LOAD_STEP_CNT);
    return 2;
}

static int lua_update(lua_State *L) {
    (void)L;  // Unused

    // Calculate delta T
    float update_et_this = pd->system->getElapsedTime();
    float update_delta_t = update_et_this - update_et_last;
    update_et_last = update_et_this;

    // Real work is done here!
    game_update(update_delta_t);

    // Show prompt
    if (gs.player_interact && !prompt_shown) {
        view_prompt_draw();
        prompt_shown = true;
    }

    if (!gs.player_interact && prompt_shown) {
        if (prompt_shown) {
            prompt_shown = false;
            screen_clear_req = true;
        }
    }

    // Clear screen when changing state!
    // Or when requested by the view
    if (player_state_old != gs.player_state || screen_clear_req) {
        pd->graphics->clear(kColorBlack);
        player_state_time = 0.0;
        screen_clear_req = false;
    }

    switch (gs.player_state) {
        case PLAYER_ACTIVE:
            view_game_draw(player_state_time, update_delta_t);
            break;
        case PLAYER_READING:
            view_note_draw(player_state_time);
            break;
        case PLAYER_KEYPAD:
            view_keypad_draw(player_state_time);
            break;
        case PLAYER_GAMEOVER:
            view_gameover_draw(player_state_time);
            break;
    }

    player_state_old = gs.player_state;
    player_state_time += update_delta_t;

#ifdef DEBUG

    // Print periodically
    if (update_cnt++ % 100 == 0) {
        print_perf();
        // TODO: Print on screen!
        debug("Charge    : %f", (double)gs.torch_charge);
        debug("Awareness : %f", (double)gs.enemy_awareness);
        debug("Aggression: %f", (double)gs.enemy_aggression);
    }
    minigl_perf_clear();

    pd->system->drawFPS(0, 0);
#endif

    return 0;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
    int eventHandler(PlaydateAPI *_pd, PDSystemEvent event, uint32_t arg) {

    (void)arg;  // Unused

    const char *err;

    switch (event) {
        case kEventInitLua:
            //---------------------------------------------------------------------------
            // LUA Init
            //---------------------------------------------------------------------------

            if (!pd->lua->addFunction(lua_reset, "resetC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
            if (!pd->lua->addFunction(lua_load, "loadC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
            if (!pd->lua->addFunction(lua_update, "updateC", &err)) {
                pd->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
            }
            break;

        case kEventInit:
            //---------------------------------------------------------------------------
            // Init
            //---------------------------------------------------------------------------

            pd = _pd;

            // Configure device
            pd->display->setRefreshRate(30);

            // int seed = time(NULL);
            int seed = 0;
            debug("SEED: %d", seed);
            srand(seed);
            break;

        case kEventTerminate:
            // Cleanup!
            break;

        default:
            break;
    }

    return 0;
}
