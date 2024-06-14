#include "map_renderer.h"

#include "cglm/affine.h"
#include "minigl/minigl.h"
#include "utils.h"

// Geometry
minigl_obj_t obj_floor;
minigl_obj_t obj_wall;
minigl_obj_t obj_statue;
minigl_obj_t obj_column;
minigl_obj_t obj_base;
minigl_obj_t obj_wetfloor;
minigl_obj_t obj_note;

// Textures
extern minigl_tex_t tex_wetfloor;
extern minigl_tex_t tex_note;
extern minigl_tex_t tex_venus[BB_SPRITE_SIZE];

// Other
float map_render_timer;

void map_renderer_init(void) {
    map_render_timer = 0;

    //---------------------------------------------------------------------------
    // Geometry
    //---------------------------------------------------------------------------

    const float WALL_SCALE_Y = 1.5f;
    const float FLOOR_Y_OFF = WALL_SCALE_Y / 2.0f;

    // Load tile object
    minigl_obj_t obj_tile;

    minigl_obj_read_file("res/models/tile.obj", &obj_tile, MINIGL_OBJ_TEXFLIPY);
    minigl_obj_read_file("res/models/wall.obj", &obj_wall, 0);
    minigl_obj_read_file("res/models/cube.obj", &obj_column, 0);
    minigl_obj_read_file("res/models/cube.obj", &obj_base, 0);
    minigl_obj_read_file("res/models/wetfloor.obj", &obj_wetfloor, 0);

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
    minigl_obj_trans(&obj_wall, trans);

    // Statue
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    glm_translate(trans, (vec3){0.0f, 0.0f, 0.0f});
    minigl_obj_copy_trans(obj_tile, trans, &obj_statue);

    // Column
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.8f, 0.0f});
    glm_scale(trans, (vec3){0.4f, 3.0, 0.4});
    minigl_obj_trans(&obj_column, trans);

    // Base
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){1.0f, 0.8, 1.0});
    glm_translate(trans, (vec3){0.0f, -0.5f, 0.0f});
    minigl_obj_trans(&obj_base, trans);

    // Wetfloor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale_uni(trans, 0.7f);
    glm_translate(trans, (vec3){0.0f, -1.2f, 0.0f});
    minigl_obj_trans(&obj_wetfloor, trans);

    // Note
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){0.3f, 0.4, 1.0});
    minigl_obj_copy_trans(obj_tile, trans, &obj_note);
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
    // Skip items marked as hidden
    if (item.hidden) return;

    mat4 item_trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(item_trans, (vec3){((float)x) + 0.5f, 0.0f, ((float)y) + 0.5f});
    int bb_tex_i = 0;
    // TODO: Generalize billboard?
    if (item.type == ITEM_STATUE) {
        vec2 dir;
        tile_dir((ivec2){x, y}, camera.pos, dir);

        float bb_tex_a = vec2_angle((vec2){0.0f, -1.0f}, dir);
        bb_tex_i = bb_tex_index(bb_tex_a, item.dir);

        mat4_billboard(camera, item_trans);
    } else {
        switch (item.dir) {
            case DIR_NORTH:
                break;
            case DIR_EAST:
                glm_rotate_at(item_trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(270), (vec3){0.0f, 1.0f, 0.0f});
                break;
            case DIR_SOUTH:
                glm_rotate_at(item_trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(180), (vec3){0.0f, 1.0f, 0.0f});
                break;
            case DIR_WEST:
                glm_rotate_at(item_trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
                break;
            default:
                break;
        }
    }

    glm_mat4_mul(trans, item_trans, item_trans);

    // FIXME: Cleanup!
    if (item.type == ITEM_FLOOR) {
        if (((x + y) % 2) == 0) {
            minigl_set_color(16);
        } else {
            minigl_set_color(24);
        }
        minigl_obj_to_objbuf_trans(obj_floor, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_STATUE) {
        minigl_set_tex(tex_venus[bb_tex_i]);
        minigl_obj_to_objbuf_trans(obj_statue, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_BASE) {
        minigl_set_color(160);
        minigl_obj_to_objbuf_trans(obj_base, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_NOTE) {
        minigl_set_tex(tex_note);
        glm_rotate_at(item_trans, (vec3){0.0f, 0.0f, 0.0f}, map_render_timer, (vec3){0.0f, 1.0f, 0.0f});
        minigl_obj_to_objbuf_trans(obj_note, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_WETFLOOR) {
        minigl_set_tex(tex_wetfloor);
        minigl_obj_to_objbuf_trans(obj_wetfloor, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_COLUMN) {
        minigl_set_color(160);
        minigl_obj_to_objbuf_trans(obj_column, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    } else if (item.type == ITEM_WALL) {
        switch (item.dir) {
            case DIR_NORTH:
                minigl_set_color(160);
                break;
            case DIR_EAST:
                minigl_set_color(128);
                break;
            case DIR_SOUTH:
                minigl_set_color(160);
                break;
            case DIR_WEST:
                minigl_set_color(128);
                break;
            case DIR_ANY:
                assert(0);
        }
        minigl_obj_to_objbuf_trans(obj_wall, item_trans, &obj_buf);
        minigl_draw(obj_buf);
    }
}

void map_renderer_draw(map_t map, mat4 trans, camera_t camera, float delta_t) {
    map_render_timer += delta_t;
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = map[y][x];
            if (map_viz_xy(map, camera.pos, x, y)) {
                for (int i = 0; i < tile.item_cnt; i++) {
                    map_item_draw(tile.items[i], camera, trans, x, y);
                }
            }
        }
    }
}
