#include "map.h"

#include "cglm/affine.h"
#include "minigl.h"
#include "object.h"
#include "texture.h"
#include "utils.h"

// Geometry
minigl_obj_t obj_floor;
minigl_obj_t obj_ceil;
minigl_obj_t obj_wall_n;
minigl_obj_t obj_wall_e;
minigl_obj_t obj_wall_s;
minigl_obj_t obj_wall_w;

// Textures
minigl_tex_t tex_floor0;
minigl_tex_t tex_wall0;
minigl_tex_t tex_ceil0;

minigl_obj_t obj_buffer;

map_t map;

LCDBitmap* minimap;
LCDBitmap* minimap_mask;
LCDBitmap* ico_player;

void minimap_item_draw(map_item_t item, int tile_x, int tile_y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    // Object
    for (int y = 0; y < MINIMAP_TILE_SIZE; y++) {
        for (int x = 0; x < MINIMAP_TILE_SIZE; x++) {
            bool draw = false;
            switch (item.type) {
                case TILE_WALL_N:
                    if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    break;
                case TILE_WALL_E:
                    if (x == 0) draw = true;
                    break;
                case TILE_WALL_S:
                    if (y == 0) draw = true;
                    break;
                case TILE_WALL_W:
                    if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    break;
                case TILE_DOOR_NS:
                    if (x == 0) draw = true;
                    if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    break;
                case TILE_DOOR_EW:
                    if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                    if (y == 0) draw = true;
                    break;
            }

            if (draw) clearpixel(bitmap_data, tile_x + x, tile_y + y, bitmap_rowbytes);
        }
    }
}

void minimap_gen(void) {
    minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
    minimap_mask = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
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

    // Load tile object
    minigl_obj_t tile_base;
    minigl_obj_read_file("res/models/tile.obj", &tile_base);

    mat4 trans;

    // Create floor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -(MAP_TILE_SIZE / 2.0f), 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_floor);

    // Create ceiling
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, (MAP_TILE_SIZE / 2.0f), 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_ceil);

    // Wall N
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, +(MAP_TILE_SIZE / 2.0f)});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(180), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_n);

    // Wall S
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, -(MAP_TILE_SIZE / 2.0f)});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_s);

    // Wall E
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-(MAP_TILE_SIZE / 2.0f), 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(270), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_e);

    // Wall W
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){+(MAP_TILE_SIZE / 2.0f), 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_w);

    //---------------------------------------------------------------------------
    // Textures
    //---------------------------------------------------------------------------

    // Load texture
    minigl_tex_read_file("res/textures/floor0.tex", &tex_floor0);
    minigl_tex_read_file("res/textures/ceil0.tex", &tex_ceil0);
    minigl_tex_read_file("res/textures/wall0.tex", &tex_wall0);

    //---------------------------------------------------------------------------
    // Map Grid
    //---------------------------------------------------------------------------

    mapgen_init(&map);
    mapgen_gen(&map);

#ifdef DEBUG
    mapgen_grid_print(map);
#endif

    //---------------------------------------------------------------------------
    // Mini map
    //---------------------------------------------------------------------------
    ico_player = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);

    minimap_gen();
}

void item_draw_floor(mat4 trans, int x, int y) {
    minigl_set_tex(tex_floor0);
    minigl_obj_transform(obj_floor, trans, &obj_buffer);
    minigl_draw(obj_buffer);
}

void map_item_draw(map_item_t item, mat4 trans, int x, int y) {
    mat4 t = GLM_MAT4_IDENTITY_INIT;
    glm_translate(t, (vec3){MAP_TILE_SIZE * x, 0.0f, MAP_TILE_SIZE * y});
    glm_mat4_mul(trans, t, t);

    switch (item.type) {
        case TILE_FLOOR:
            minigl_set_tex(tex_floor0);
            minigl_obj_transform(obj_floor, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_N:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_n, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_E:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_e, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_S:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_s, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_W:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_w, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_DOOR_NS:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_w, t, &obj_buffer);
            minigl_draw(obj_buffer);
            minigl_obj_transform(obj_wall_e, t, &obj_buffer);
            minigl_draw(obj_buffer);
            item_draw_floor(t, x, y);
            break;
        case TILE_DOOR_EW:
            minigl_set_tex(tex_wall0);
            minigl_obj_transform(obj_wall_n, t, &obj_buffer);
            minigl_draw(obj_buffer);
            minigl_obj_transform(obj_wall_s, t, &obj_buffer);
            minigl_draw(obj_buffer);

            item_draw_floor(t, x, y);
            break;
    }
}

void map_draw(mat4 trans, camera_t camera) {
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            map_tile_t tile = map.grid[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                map_item_draw(tile.items[i], trans, x, y);
            }
        }
    }
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
