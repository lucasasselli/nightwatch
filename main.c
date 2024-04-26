#include <stdio.h>
#include <stdlib.h>

#include "game.h"
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
minigl_obj_t geometry_buffer;  // FIXME: We need some way to generate it behind the scene
minigl_tex_t tex_my;
minigl_tex_t tex_dither;

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

    // Prepare the transformation matrix
    mat4 trans;
    glm_mat4_mul(proj, view, trans);

    // Shade the vertices!
    // TODO: Simplify and wrap
    for (int i = 0; i < obj_my.vcoord_size; i++) {
        // Apply transformation
        glm_mat4_mulv(trans, obj_my.vcoord_ptr[i], geometry_buffer.vcoord_ptr[i]);

        // Convert to carthesian coord.
        glm_vec3_divs(geometry_buffer.vcoord_ptr[i], geometry_buffer.vcoord_ptr[i][3], geometry_buffer.vcoord_ptr[i]);
    }

    minigl_clear(0, -1.0f);
    minigl_draw(geometry_buffer);
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

        // Load model
        minigl_obj_read_file("res/models/cube.obj", &obj_my);

        minigl_tex_read_file("res/textures/cube.tex", &tex_my);
        minigl_tex_read_file("res/dither/bayer16tile2.tex", &tex_dither);

        minigl_set_dither(tex_dither);
        minigl_set_tex(tex_my);

        // Create a buffer for processed geometry
        geometry_buffer = obj_my;
        geometry_buffer.vcoord_ptr = (vec4 *)malloc(sizeof(vec4) * obj_my.vcoord_size);

        // Load fonts
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
