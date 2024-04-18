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
camera_t camera;

// Resources
LCDFont *font = NULL;
minigl_obj_t obj_cube;
minigl_texture_t tex_mario;

#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorBlack : kColorWhite)

minigl_texture_t texture_read(char *path) {
    const char *error = NULL;
    LCDBitmap *bm = pd->graphics->loadBitmap(path, &error);
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
        vec3 camera_delta;
        if (pushed & kButtonUp) {
            glm_vec3_scale_as(camera.front, INPUT_CAMERA_TSPEED, camera_delta);
            glm_vec3_add(camera.pos, camera_delta, camera.pos);
        }
        if (pushed & kButtonDown) {
            glm_vec3_scale_as(camera.front, INPUT_CAMERA_TSPEED, camera_delta);
            glm_vec3_sub(camera.pos, camera_delta, camera.pos);
        }
        if (pushed & kButtonRight) {
            camera.yaw += INPUT_CAMERA_RSPEED;
        }
        if (pushed & kButtonLeft) {
            camera.yaw -= INPUT_CAMERA_RSPEED;
        }
        if (pushed & kButtonA) {
            camera.pitch += INPUT_CAMERA_RSPEED;
        }
        if (pushed & kButtonB) {
            camera.pitch -= INPUT_CAMERA_RSPEED;
        }

        vec3 direction;
        direction[0] = cosf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
        direction[1] = sinf(glm_rad(camera.pitch));
        direction[2] = sinf(glm_rad(camera.yaw)) * cosf(glm_rad(camera.pitch));
        glm_vec3_normalize_to(direction, camera.front);

        // Log camera
        pd->system->logToConsole("Camera pos: %f %f %f", camera.pos[0], camera.pos[1], camera.pos[2]);
        pd->system->logToConsole("Camera dir: %f %f %f", camera.front[0], camera.front[1], camera.front[2]);
    }

    pd->graphics->clear(kColorWhite);

    minigl_obj_t obj = obj_cube;

    // Copy the vertices
    obj.vcoord_ptr = (vec4 *)malloc(sizeof(vec4) * obj_cube.vcoord_size);
    memcpy(obj.vcoord_ptr, obj_cube.vcoord_ptr, sizeof(vec4) * obj_cube.vcoord_size);

    mat4 proj;
    mat4 view;
    vec3 camera_center;

    glm_perspective(glm_rad(60), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), 0.1f, 10.0f, proj);
    glm_vec3_add(camera.pos, camera.front, camera_center);
    glm_lookat(camera.pos, camera_center, camera.up, view);

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
    minigl_set_texture(tex_mario);  // TODO: Move
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

        // Set camera
        camera.yaw = -90.0;
        camera.pitch = 0.0;
        glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, camera.pos);
        glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);
        glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);

        pd->system->logToConsole("Setup complete!");
    }

    if (event == kEventKeyPressed) {
        // TODO:
    }

    return 0;
}
