#include "renderer.h"

#include "game_state.h"
#include "minigl/minigl.h"
#include "utils.h"

// Other
float render_timer = 0;

static void mat4_billboard(camera_t camera, mat4 trans) {
    vec2 poly_dir2;
    poly_dir2[0] = 0.0f;
    poly_dir2[1] = -1.0f;
    // glm_vec2_normalize(poly_dir2);

    float a = vec2_angle(poly_dir2, camera.front);
    glm_rotate_at(trans, GLM_VEC3_ZERO, -a, (vec3){0.0f, 1.0f, 0.0f});
}

static void draw_item(item_t* item, camera_t camera, mat4 trans, int x, int y) {
    // Skip items marked as hidden
    if (item->hidden) return;

    mat4 item_trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(item_trans, (vec3){((float)x) + 0.5f, 0.0f, ((float)y) + 0.5f});

    switch (item->type) {
        case ITEM_NORMAL:
        case ITEM_FLOOR:
        case ITEM_WALL:
            switch (item->dir) {
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
            break;
        case ITEM_BILLBOARD:
            mat4_billboard(camera, item_trans);
            break;
    }

    vec2 dir;
    float a;
    switch (item->tex_mode) {
        case TEX_MODE_NONE:
            minigl_set_color(item->color);
            break;

        case TEX_MODE_IMAGE:
            minigl_set_tex(*tex_get(item->tex_id));
            break;

        case TEX_MODE_MDBB:
            tile_dir((ivec2){x, y}, camera.pos, dir);

            a = vec2_angle((vec2){0.0f, -1.0f}, dir) + item->dir * GLM_PI_2f;
            minigl_set_tex(*tex_mdbb_get_a(item->tex_mdbb_id, a));
            break;
    }

    glm_mat4_mul(trans, item_trans, item_trans);

    if (item->effects & EFFECT_SPIN) {
        glm_rotate_at(item_trans, (vec3){0.0f, 0.0f, 0.0f}, render_timer, (vec3){0.0f, 1.0f, 0.0f});
    }

    minigl_obj_to_objbuf_trans(*obj_get(item->obj_id), item_trans, &obj_buf);
    minigl_draw(obj_buf);
}

void renderer_draw(game_state_t* gs, float delta_t) {
    render_timer += delta_t;

    // Draw map
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = gs->map[y][x];
            if (map_viz_xy(gs->map, gs->camera.pos, x, y)) {
                item_t* item = tile.items;
                while (item != NULL) {
                    draw_item(item, gs->camera, gs->camera.trans, x, y);
                    item = item->next;
                }
            }
        }
    }

    // Draw enemy
    if (gs->enemy_state != ENEMY_HIDDEN) {
        mat4 enemy_trans = GLM_MAT4_IDENTITY_INIT;
        vec2 enemy_pos;
        ivec2_to_vec2_center(gs->enemy_tile, enemy_pos);
        glm_translate(enemy_trans, CAMERA_VEC3(enemy_pos));
        mat4_billboard(gs->camera, enemy_trans);
        glm_mat4_mul(gs->camera.trans, enemy_trans, enemy_trans);

        minigl_obj_to_objbuf_trans(*obj_get(OBJ_ID_ENEMY), enemy_trans, &obj_buf);
        minigl_set_tex(*tex_get(TEX_ID_ENEMY));
        minigl_draw(obj_buf);
    }
}
