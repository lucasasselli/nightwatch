#include "map_renderer.h"

#include "cglm/affine.h"
#include "minigl.h"
#include "object.h"
#include "texture.h"
#include "utils.h"

#define BB_SPRITE_SIZE 36

// Geometry
minigl_obj_t obj_light;
minigl_obj_t obj_wall_n;
minigl_obj_t obj_wall_e;
minigl_obj_t obj_wall_s;
minigl_obj_t obj_wall_w;
minigl_obj_t obj_statue;

// Textures
minigl_tex_t tex_venus[BB_SPRITE_SIZE];

minigl_obj_buf_t buf;

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
                case TILE_STATUE:
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
    buf = minigl_obj_buf_init(50);

    //---------------------------------------------------------------------------
    // Geometry
    //---------------------------------------------------------------------------

    const float WALL_Y_OFF = 1.0f;

    // Load tile object
    minigl_obj_t obj_tile;
    minigl_obj_t obj_wall;

    minigl_obj_read_file("res/models/tile.obj", &obj_tile);
    minigl_obj_read_file("res/models/wall.obj", &obj_wall);

    mat4 trans;

    // Create ceiling
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, MAP_TILE_SIZE + WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    minigl_obj_copy_trans(obj_tile, trans, &obj_light);

    // Wall N
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, WALL_Y_OFF, -(MAP_TILE_SIZE / 2.0f)});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_n);

    // Wall S
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, WALL_Y_OFF, +(MAP_TILE_SIZE / 2.0f)});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(180), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_s);

    // Wall E
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){+(MAP_TILE_SIZE / 2.0f), WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(270), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_e);

    // Wall W
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-(MAP_TILE_SIZE / 2.0f), WALL_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(obj_wall, trans, &obj_wall_w);

    // Statue
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, WALL_Y_OFF, 0.0f});
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    glm_scale_uni(trans, MAP_TILE_SIZE);
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

float vec2_angle(vec2 a, vec2 b) {
    float dot = glm_vec2_dot(a, b);
    float det = a[0] * b[1] - a[1] * b[0];
    return atan2f(det, dot);
}

void vec2_pos_to_tile(int x, int y, vec3 pos, vec2 out) {
    out[0] = (((float)x) + 0.5f) * MAP_TILE_SIZE - pos[0];
    out[1] = (((float)y) + 0.5f) * MAP_TILE_SIZE - pos[2];
    glm_vec2_normalize(out);
}

void map_item_draw(map_item_t item, minigl_camera_t camera, mat4 trans, int x, int y) {
    mat4 t = GLM_MAT4_IDENTITY_INIT;
    glm_translate(t, (vec3){MAP_TILE_SIZE * x, 0.0f, MAP_TILE_SIZE * y});

    int bb_tex_i = 0;
    if (item.type == TILE_STATUE) {
        vec2 camera_dir2;
        camera_dir2[0] = camera.front[0];
        camera_dir2[1] = camera.front[2];
        // glm_vec2_normalize(camera_dir2);

        // FIXME:
        vec2 poly_dir2;
        poly_dir2[0] = 0.0f;
        poly_dir2[1] = -1.0f;
        // glm_vec2_normalize(poly_dir2);

        vec2 pos_to_tile_dir;
        vec2_pos_to_tile(x, y, camera.pos, pos_to_tile_dir);

        float bb_poly_a = vec2_angle(poly_dir2, camera_dir2);
        float bb_tex_a = vec2_angle(poly_dir2, pos_to_tile_dir);

        glm_rotate_at(t, (vec3){0.0f, 0.0f, 0.0f}, -bb_poly_a, (vec3){0.0f, 1.0f, 0.0f});

        glm_make_deg(&bb_tex_a);
        if (bb_tex_a > 0) {
            bb_tex_i = (int)(fabs(bb_tex_a) / 10);
        } else {
            bb_tex_i = (BB_SPRITE_SIZE - 1) - (int)(fabs(bb_tex_a) / 10);
        }

        assert(bb_tex_i >= 0 && bb_tex_i < BB_SPRITE_SIZE);
    }
    glm_mat4_mul(trans, t, t);

    switch (item.type) {
        case TILE_FLOOR:
            // Draw light
            minigl_set_color(255);
            minigl_obj_to_obj_buf_trans(obj_light, t, &buf);
            minigl_draw(buf);
            break;
        case TILE_STATUE:
            minigl_set_tex(tex_venus[bb_tex_i]);
            minigl_obj_to_obj_buf_trans(obj_statue, t, &buf);
            minigl_draw(buf);
            break;
        case TILE_WALL_N:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(128);
            minigl_obj_to_obj_buf_trans(obj_wall_n, t, &buf);
            minigl_draw(buf);
            break;
        case TILE_WALL_E:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(96);
            minigl_obj_to_obj_buf_trans(obj_wall_e, t, &buf);
            minigl_draw(buf);
            break;
        case TILE_WALL_S:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(128);
            minigl_obj_to_obj_buf_trans(obj_wall_s, t, &buf);
            minigl_draw(buf);
            break;
        case TILE_WALL_W:
            // minigl_set_tex(tex_wall0);
            minigl_set_color(96);
            minigl_obj_to_obj_buf_trans(obj_wall_w, t, &buf);
            minigl_draw(buf);
            break;
    }
}

void map_draw(map_t map, mat4 trans, minigl_camera_t camera) {
    vec2 camera_dir;
    camera_dir[0] = camera.front[0];
    camera_dir[1] = camera.front[2];
    glm_vec2_normalize(camera_dir);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // NOTE: Use the cross product of the position-tile vector and the
            // camera direction to check if the tile is within the FOV.
            vec2 pos_to_tile_dir;
            vec2_pos_to_tile(x, y, camera.pos, pos_to_tile_dir);

            float a = glm_vec2_dot(pos_to_tile_dir, camera_dir);

            // NOTE: FOV should be divided by two, but since tile are discrete,
            // we need to use a larger angle
            // FIXME: There's something wrong!!!
            // if (a >= cosf(glm_rad(CAMERA_FOV))) {
            map_tile_t tile = map[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                map_item_draw(tile.items[i], camera, trans, x, y);
            }
            //}
        }
    }
}

void minimap_draw(int x, int y, minigl_camera_t camera) {
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
