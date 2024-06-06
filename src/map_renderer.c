#include "map_renderer.h"

#include "cglm/affine.h"
#include "game.h"
#include "minigl/minigl.h"
#include "utils.h"

#define BB_SPRITE_SIZE 36

// Geometry
minigl_obj_t obj_floor;
minigl_obj_t obj_wall_n;
minigl_obj_t obj_wall_e;
minigl_obj_t obj_wall_s;
minigl_obj_t obj_wall_w;
minigl_obj_t obj_statue;

// Textures
minigl_tex_t tex_venus[BB_SPRITE_SIZE];

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
            if (item.type == ITEM_WALL) {
                switch (item.dir) {
                    case DIR_NORTH:
                        if (y == 0) draw = true;
                        break;
                    case DIR_EAST:
                        if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                        break;
                    case DIR_SOUTH:
                        if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                        break;
                    case DIR_WEST:
                        if (x == 0) draw = true;
                        break;
                    case DIR_ANY:
                        assert(0);  // Not allowed
                        break;
                }
            } else if (item.type == ITEM_STATUE) {
                if (y == 0) draw = true;
                if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                if (x == 0) draw = true;
                break;
            }

            if (draw) clearpixel(bitmap_data, tile_x + x, tile_y + y, bitmap_rowbytes);
        }
    }
}

void minimap_draw(int x, int y, game_state_t* state) {
    int off_x = ((int)state->camera.pos[0]) - MINIMAP_SIZE_X / 2;
    int off_y = ((int)state->camera.pos[1]) - MINIMAP_SIZE_Y / 2;

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
    pd->graphics->drawRotatedBitmap(ico_player, x + MINIMAP_SIZE_X / 2, y + MINIMAP_SIZE_Y / 2, state->camera.yaw + 90, 0.5f, 0.5f, 0.2f, 0.2f);
}

void minimap_gen(map_t map) {
    minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
    minimap_mask = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = map[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                minimap_item_draw(tile.items[i], MINIMAP_TILE_SIZE * x, MINIMAP_TILE_SIZE * y, bitmap_data, bitmap_rowbytes);
            }
        }
    }
}

void map_init(void) {
    //---------------------------------------------------------------------------
    // Geometry
    //---------------------------------------------------------------------------

    const float WALL_SCALE_Y = 1.5f;
    const float FLOOR_Y_OFF = WALL_SCALE_Y / 2.0f;

    // Load tile object
    minigl_obj_t obj_tile;
    minigl_obj_t obj_wall;

    minigl_obj_read_file("res/models/tile.obj", &obj_tile);
    minigl_obj_read_file("res/models/wall.obj", &obj_wall);

    mat4 trans;

    // Create floor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -FLOOR_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    minigl_obj_copy_trans(obj_tile, trans, &obj_floor);

    // Wall N
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, -0.5f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_n);

    // Wall S
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, +0.5f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(180), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_s);

    // Wall E
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){+0.5f, 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(270), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_e);

    // Wall W
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.5f, 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_w);

    // Statue
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    glm_translate(trans, (vec3){0.0f, 0.0f, 0.0f});
    minigl_obj_copy_trans(obj_tile, trans, &obj_statue);

    //---------------------------------------------------------------------------
    // Textures
    //---------------------------------------------------------------------------

    char tex_path[50];

    for (int i = 0; i < BB_SPRITE_SIZE; i++) {
        sprintf(tex_path, "res/sprites/venus/venus_%02d.tex", i);
        minigl_tex_read_file(tex_path, &tex_venus[i]);
    }
}

int bb_tex_index(float a, map_item_dir_t dir) {
    glm_make_deg(&a);

    a += 90.0f * dir;

    int a_int = ((int)fabsf(a)) % 360;

    if (a > 0) {
        return a_int / 10;
    } else {
        return (BB_SPRITE_SIZE - 1) - a_int / 10;
    }
}

void map_item_draw(map_item_t item, camera_t camera, mat4 trans, int x, int y) {
    mat4 tile_trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(tile_trans, (vec3){((float)x) + 0.5f, 0.0f, ((float)y) + 0.5f});

    int bb_tex_i = 0;
    if (item.type == ITEM_STATUE) {
        vec2 dir;
        tile_dir((ivec2){x, y}, camera.pos, dir);

        float bb_tex_a = vec2_angle((vec2){0.0f, -1.0f}, dir);
        bb_tex_i = bb_tex_index(bb_tex_a, item.dir);

        mat4_billboard(camera, tile_trans);
    }
    glm_mat4_mul(trans, tile_trans, tile_trans);

    if (item.type == ITEM_FLOOR) {
        if (((x + y) % 2) == 0) {
            minigl_set_color(16);
        } else {
            minigl_set_color(24);
        }
        minigl_obj_to_objbuf_trans(obj_floor, tile_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_STATUE) {
        minigl_set_tex(tex_venus[bb_tex_i]);
        minigl_obj_to_objbuf_trans(obj_statue, tile_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_WALL) {
        switch (item.dir) {
            case DIR_NORTH:
                minigl_set_color(160);
                minigl_obj_to_objbuf_trans(obj_wall_n, tile_trans, &obj_buf);
                break;
            case DIR_EAST:
                minigl_set_color(128);
                minigl_obj_to_objbuf_trans(obj_wall_e, tile_trans, &obj_buf);
                break;
            case DIR_SOUTH:
                minigl_set_color(160);
                minigl_obj_to_objbuf_trans(obj_wall_s, tile_trans, &obj_buf);
                break;
            case DIR_WEST:
                minigl_set_color(128);
                minigl_obj_to_objbuf_trans(obj_wall_w, tile_trans, &obj_buf);
                break;
            case DIR_ANY:
                assert(0);
        }
        minigl_draw(obj_buf);
    }
}

void map_draw(map_t map, mat4 trans, camera_t camera) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = map[y][x];
            if (tile.visible) {
                for (int i = 0; i < tile.item_cnt; i++) {
                    map_item_draw(tile.items[i], camera, trans, x, y);
                }
            }
        }
    }
}
