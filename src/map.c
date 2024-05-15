#include "map.h"

#include "cglm/affine.h"

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
    glm_translate(trans, (vec3){0.0f, 0.0f, -(MAP_TILE_SIZE / 2.0f)});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_n);

    // Wall S
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, +(MAP_TILE_SIZE / 2.0f)});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_s);

    // Wall E
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-(MAP_TILE_SIZE / 2.0f), 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_e);

    // Wall W
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){+(MAP_TILE_SIZE / 2.0f), 0.0f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &obj_wall_w);

    //---------------------------------------------------------------------------
    // Texttures
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
}

void item_draw_floor(mat4 trans, int x, int y) {
    minigl_set_tex(tex_floor0);
    minigl_obj_transform(obj_floor, trans, &obj_buffer);
    minigl_draw(obj_buffer);
}

void item_draw(map_item_t item, mat4 trans, int x, int y) {
    mat4 t = GLM_MAT4_IDENTITY_INIT;
    glm_translate(t, (vec3){MAP_TILE_SIZE * x, 0.0f, MAP_SIZE * MAP_TILE_SIZE - MAP_TILE_SIZE * y});
    glm_mat4_mul(trans, t, t);

    // Texture
    switch (item.type) {
        case TILE_FLOOR:
            minigl_set_tex(tex_floor0);
            break;
        case TILE_WALL_N:
        case TILE_WALL_E:
        case TILE_WALL_S:
        case TILE_WALL_W:
            minigl_set_tex(tex_wall0);
            break;
    }

    // Object
    switch (item.type) {
        case TILE_FLOOR:
            minigl_obj_transform(obj_floor, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_N:
            minigl_obj_transform(obj_wall_n, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_E:
            minigl_obj_transform(obj_wall_e, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_S:
            minigl_obj_transform(obj_wall_s, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_WALL_W:
            minigl_obj_transform(obj_wall_w, t, &obj_buffer);
            minigl_draw(obj_buffer);
            break;
        case TILE_DOOR_NS:
            item_draw_floor(t, x, y);
            break;
        case TILE_DOOR_EW:
            item_draw_floor(t, x, y);
            break;
    }
}

void map_draw(mat4 trans) {
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            map_tile_t tile = map.grid[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                item_draw(tile.items[i], trans, x, y);
            }
        }
    }
}
