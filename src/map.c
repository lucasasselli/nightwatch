#include "map.h"

#include "cglm/affine.h"

minigl_obj_t _obj_tile_floor;
minigl_obj_t _obj_tile_ceil;
minigl_obj_t _obj_tile_wall_ns;
minigl_obj_t _obj_tile_wall_ew;

minigl_tex_t _tex_tile_floor0;
minigl_tex_t _tex_tile_wall0;
minigl_tex_t _tex_tile_ceil0;

minigl_obj_t** _map_poly_floor;
minigl_obj_t** _map_poly_wall;
minigl_obj_t** _map_poly_ceil;

minigl_obj_t _obj_buf;

map_t map;

void map_init(void) {
    // Allocate memory for geometry
    _map_poly_floor = malloc(MAP_SIZE * sizeof(minigl_obj_t*));
    _map_poly_wall = malloc(MAP_SIZE * sizeof(minigl_obj_t*));
    _map_poly_ceil = malloc(MAP_SIZE * sizeof(minigl_obj_t*));
    for (int y = 0; y < MAP_SIZE; y++) {
        _map_poly_floor[y] = malloc(MAP_SIZE * sizeof(minigl_obj_t));
        _map_poly_wall[y] = malloc(MAP_SIZE * sizeof(minigl_obj_t));
        _map_poly_ceil[y] = malloc(MAP_SIZE * sizeof(minigl_obj_t));
        for (int x = 0; x < MAP_SIZE; x++) {
            _map_poly_floor[y][x].face_size = 0;
            _map_poly_floor[y][x].vcoord_size = 0;
            _map_poly_wall[y][x].face_size = 0;
            _map_poly_wall[y][x].vcoord_size = 0;
            _map_poly_ceil[y][x].face_size = 0;
            _map_poly_ceil[y][x].vcoord_size = 0;
        }
    }

    // FIXME: hardcoded buffer
    _obj_buf.vcoord_ptr = (vec4*)malloc(sizeof(vec4) * 50);

    // Load tile object
    minigl_obj_t tile_base;
    minigl_obj_read_file("res/models/tile.obj", &tile_base);

    mat4 trans;

    // Create floor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -(MAP_TILE_SIZE / 2.0f), 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &_obj_tile_floor);

    // Create ceiling
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, (MAP_TILE_SIZE / 2.0f), 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &_obj_tile_ceil);

    // Create wall NS
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &_obj_tile_wall_ns);

    // Create wall EW
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale_uni(trans, MAP_TILE_SIZE);
    minigl_obj_copy_trans(tile_base, trans, &_obj_tile_wall_ew);

    // Load texture
    minigl_tex_read_file("res/textures/floor0.tex", &_tex_tile_floor0);
    minigl_tex_read_file("res/textures/ceil0.tex", &_tex_tile_ceil0);
    minigl_tex_read_file("res/textures/wall0.tex", &_tex_tile_wall0);
}

void map_gen_grid(void) {
    unsigned int seed = time(NULL);

    debug("Map seed: %d\n", seed);

    srand(seed);

    mapgen_init(&map);
    mapgen_gen(&map);

#ifdef DEBUG
    mapgen_grid_print(map);
#endif
}

void place_poly_at(minigl_obj_t src_obj, minigl_obj_t** grid, int x, int y) {
    minigl_obj_t* out_obj = &grid[y][x];
    minigl_obj_copy(src_obj, out_obj);

    for (int i = 0; i < out_obj->vcoord_size; i++) {
        vec4 trasl;
        glm_vec4_copy((vec4){MAP_TILE_SIZE * x, 0.0f, MAP_SIZE * MAP_TILE_SIZE - MAP_TILE_SIZE * y, 0.0f}, trasl);
        glm_vec4_add(out_obj->vcoord_ptr[i], trasl, out_obj->vcoord_ptr[i]);
    }
}

void map_gen_poly(void) {
    // Pracalculate the tiles content
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // Draw floor
            if (map.grid[y][x] != TILE_EMPTY) {
                place_poly_at(_obj_tile_floor, _map_poly_floor, x, y);
                place_poly_at(_obj_tile_ceil, _map_poly_ceil, x, y);
            }

            // Draw walls
            if (map.grid[y][x] == TILE_WALL_NS) {
                place_poly_at(_obj_tile_wall_ns, _map_poly_wall, x, y);
            }

            if (map.grid[y][x] == TILE_WALL_EW) {
                place_poly_at(_obj_tile_wall_ew, _map_poly_wall, x, y);
            }
        }
    }
}

void map_draw(mat4 trans) {
    // FIXME: Draw more tiles in one go!
    // Draw walls
    minigl_set_tex(_tex_tile_wall0);
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            minigl_obj_transform(_map_poly_wall[y][x], trans, &_obj_buf);
            minigl_draw(_obj_buf);
        }
    }

    // Draw floor
    minigl_set_tex(_tex_tile_floor0);
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            minigl_obj_transform(_map_poly_floor[y][x], trans, &_obj_buf);
            minigl_draw(_obj_buf);
        }
    }

    // Draw ceiling
    minigl_set_tex(_tex_tile_ceil0);
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            minigl_obj_transform(_map_poly_ceil[y][x], trans, &_obj_buf);
            minigl_draw(_obj_buf);
        }
    }
}
