#include "map.h"

#include "cglm/affine.h"
#include "minigl.h"
#include "object.h"
#include "texture.h"
#include "utils.h"

// Geometry
minigl_obj_t obj_light;
minigl_obj_t obj_wall_n;
minigl_obj_t obj_wall_e;
minigl_obj_t obj_wall_s;
minigl_obj_t obj_wall_w;
minigl_obj_t obj_base;

// Textures
minigl_tex_t tex_floor0;
minigl_tex_t tex_wall0;
minigl_tex_t tex_ceil0;

minigl_obj_t obj_buffer;

LCDBitmap* minimap;
LCDBitmap* minimap_mask;
LCDBitmap* ico_player;

void minimap_init(void) {
    ico_player = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);
}

void minimap_item_draw(map_item_t item, int tile_x, int tile_y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    // Object
    for (int y = 0; y < MINIMAP_TILE_SIZE; y++) {
        for (int x = 0; x < MINIMAP_TILE_SIZE; x++) {
            bool draw = false;
            switch (item.type) {
                case TILE_WALL_N:
                    if (y == 0) draw = true;
                    break;
                case TILE_WALL_E:
                    if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    break;
                case TILE_WALL_S:
                    if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    break;
                case TILE_WALL_W:
                    if (x == 0) draw = true;
                    break;
            }

            if (draw) clearpixel(bitmap_data, tile_x + x, tile_y + y, bitmap_rowbytes);
        }
    }
}

void minimap_gen(map_t map) {
    minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
    minimap_mask = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = map.grid[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                minimap_item_draw(tile.items[i], MINIMAP_TILE_SIZE * x, MINIMAP_TILE_SIZE * y, bitmap_data, bitmap_rowbytes);
            }
        }
    }
}

void map_init(void) {
    // FIXME: hardcoded buffer
    obj_buffer.vcoord_ptr = (vec4*)malloc(sizeof(vec4) * 50);

    //---------------------------------------------------------------------------
    // Geometry
    //---------------------------------------------------------------------------

    const float WALL_Y_OFF = 1.0f;

    // Load tile object
    minigl_obj_t tile_base;
    minigl_obj_read_file("res/models/tile.obj", &tile_base);
    minigl_obj_read_file("res/models/cube.obj", &obj_base);

    mat4 trans;

    // Create ceiling
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, MAP_TILE_SIZE + WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    // glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_light);

    // Wall N
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, WALL_Y_OFF, -(MAP_TILE_SIZE / 2.0f)});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_n);

    // Wall S
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, WALL_Y_OFF, +(MAP_TILE_SIZE / 2.0f)});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(180), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_s);

    // Wall E
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){+(MAP_TILE_SIZE / 2.0f), WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(270), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_e);

    // Wall W
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-(MAP_TILE_SIZE / 2.0f), WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_w);

    // Base
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -1.25f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.0f, 1.0});
    minigl_obj_transform(obj_base, trans, &obj_base);

    //---------------------------------------------------------------------------
    // Textures
    //---------------------------------------------------------------------------

    // Load texture
    minigl_tex_read_file("res/textures/floor0.tex", &tex_floor0);
    minigl_tex_read_file("res/textures/ceil0.tex", &tex_ceil0);
    minigl_tex_read_file("res/textures/wall0.tex", &tex_wall0);
}

void map_item_draw(map_item_t item, mat4 trans, int x, int y) {
    mat4 t = GLM_MAT4_IDENTITY_INIT;
    glm_translate(t, (vec3){MAP_TILE_SIZE * x, 0.0f, MAP_TILE_SIZE * y});
    glm_mat4_mul(trans, t, t);

    switch (item.type) {
        case TILE_FLOOR:
            // Draw light
            minigl_set_color(255);
            minigl_obj_transform(obj_light, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_BASE:
            minigl_set_color(255);
            minigl_obj_transform(obj_base, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_N:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(128);
            minigl_obj_transform(obj_wall_n, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_E:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(96);
            minigl_obj_transform(obj_wall_e, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_S:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(128);
            minigl_obj_transform(obj_wall_s, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_W:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(96);
            minigl_obj_transform(obj_wall_w, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
    }
}

void map_draw(map_t map, mat4 trans, camera_t camera) {
    ivec2 x_range;
    x_range[0] = (int)(camera.pos[0] / MAP_TILE_SIZE) - MAP_DRAW_SIZE / 2;
    x_range[1] = x_range[0] + MAP_DRAW_SIZE;
    glm_ivec2_clamp(x_range, 0, MAP_SIZE);

    ivec2 y_range;
    y_range[0] = (int)(camera.pos[2] / MAP_TILE_SIZE) - MAP_DRAW_SIZE / 2;
    y_range[1] = y_range[0] + MAP_DRAW_SIZE;
    glm_ivec2_clamp(y_range, 0, MAP_SIZE);

    vec2 d;
    d[0] = camera.front[0];
    d[1] = camera.front[2];
    // glm_vec2_normalize(d);

    for (int y = y_range[0]; y < y_range[1]; y++) {
        for (int x = x_range[0]; x < x_range[1]; x++) {
            // NOTE: Use the cross product of the position-tile vector and the
            // camera direction to check if the tile is within the FOV.
            vec2 t;
            t[0] = x * MAP_TILE_SIZE - camera.pos[0];
            t[1] = y * MAP_TILE_SIZE - camera.pos[2];
            glm_vec2_normalize(t);

            float a = glm_vec2_dot(t, d);

            // NOTE: FOV should be divided by two, but since tile are discrete,
            // we need to use a larger angle
            if (a >= cosf(glm_rad(CAMERA_FOV))) {
                map_tile_t tile = map.grid[y][x];
                for (int i = 0; i < tile.item_cnt; i++) {
                    map_item_draw(tile.items[i], trans, x, y);
                }
            }
        }
    }

    // TODO: Draw one big floor!
    // Draw floor
    /*
    minigl_set_tex(tex_floor0);
    minigl_obj_transform(obj_floor, trans, &obj_buffer);
    minigl_draw(obj_buffer);
    */
}

void minimap_draw(int x, int y, camera_t camera) {
    int off_x = ((int)camera.pos[0]) - MINIMAP_SIZE_X / 2;
    int off_y = ((int)camera.pos[2]) - MINIMAP_SIZE_Y / 2;

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->clearBitmap(minimap_mask, kColorBlack);
    pd->graphics->getBitmapData(minimap_mask, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = off_y; y < off_y + MINIMAP_SIZE_Y; y++) {
        for (int x = off_x; x < off_x + MINIMAP_SIZE_X; x++) {
            if (x < MAP_SIZE * MINIMAP_TILE_SIZE && y < MAP_SIZE * MINIMAP_TILE_SIZE) {
                clearpixel(bitmap_data, x, y, bitmap_rowbytes);
            }
        }
    }
    pd->graphics->setBitmapMask(minimap, minimap_mask);

    pd->graphics->drawBitmap(minimap, x - off_x, y - off_y, kBitmapUnflipped);
    pd->graphics->drawRect(x, y, MINIMAP_SIZE_X, MINIMAP_SIZE_Y, kColorWhite);
    pd->graphics->drawRotatedBitmap(ico_player, x + MINIMAP_SIZE_X / 2, y + MINIMAP_SIZE_Y / 2, camera.yaw + 90, 0.5f, 0.5f, 0.2f, 0.2f);
}
