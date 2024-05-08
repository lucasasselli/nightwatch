#include "map.h"

#include "cglm/affine.h"

minigl_obj_t _obj_tile_floor;
minigl_obj_t _obj_tile_wall0;
minigl_obj_t _obj_tile_wall1;

minigl_tex_t _tex_tile_floor0;

minigl_obj_t _map_poly_floor[MAP_GRID_SIZE][MAP_GRID_SIZE];

minigl_obj_t _obj_buf;

// clang-format off
char map_tile[4][4] = {
    {'+', '-', '-', '+' },
    {'|', ' ', ' ', '|' },
    {'|', ' ', ' ', '|' },
    {'+', '-', '-', '+' }
};
// clang-format on

void map_init(void) {
    // Load tile object
    minigl_obj_read_file("res/models/tile.obj", &_obj_tile_wall0);

    // Create floor
    mat4 trans = GLM_MAT4_IDENTITY_INIT;
    // glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    glm_translate(trans, (vec3){0.0f, -0.5f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(_obj_tile_wall0, trans, &_obj_tile_floor);

    // Load texture
    minigl_tex_read_file("res/textures/test.tex", &_tex_tile_floor0);

    // FIXME: hardcoded buffer
    _obj_buf.vcoord_ptr = (vec4*)malloc(sizeof(vec4) * 50);
}

void map_generate(void) {
    // Create tiles
    for (int y = 0; y < MAP_GRID_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_GRID_DRAW_SIZE; x++) {
            // TODO: what tile is this?

            minigl_obj_t* obj = &_map_poly_floor[y][x];

            // Draw floor
            minigl_obj_copy(_obj_tile_floor, obj);

            for (int i = 0; i < obj->vcoord_size; i++) {
                vec4 trasl;
                glm_vec4_copy((vec4){MAP_TILE_SIZE * x, 0.0f, MAP_GRID_SIZE * MAP_TILE_SIZE - MAP_TILE_SIZE * y, 0.0f}, trasl);
                glm_vec4_add(obj->vcoord_ptr[i], trasl, obj->vcoord_ptr[i]);
            }
            // Draw walls
            // Draw ceiling
        }
    }
}

void map_draw(mat4 trans) {
    // Draw floor
    minigl_set_tex(_tex_tile_floor0);
    for (int y = 0; y < MAP_GRID_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_GRID_DRAW_SIZE; x++) {
            minigl_obj_transform(_map_poly_floor[y][x], trans, &_obj_buf);
            minigl_draw(_obj_buf);
        }
    }
}
