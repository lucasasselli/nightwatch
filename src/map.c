#include "map.h"

#include <assert.h>

#include "constants.h"
#include "utils.h"

map_tile_t map_get_tile_xy(map_t *map, int x, int y) {
    assert(x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE);

    return map->grid[y][x];
}

map_tile_t map_get_tile_ivec2(map_t *map, ivec2 pos) {
    return map_get_tile_xy(map, pos[0], pos[1]);
}

map_tile_t map_get_tile_vec2(map_t *map, vec2 pos) {
    return map_get_tile_xy(map, pos[0], pos[1]);
}

item_t *tile_find_item(map_tile_t tile, int item_type, int dir, int action_type) {
    item_t *item = tile.items;
    while (item != NULL) {
        bool match = true;
        if (item_type >= 0) match &= ((int)item->type == item_type);
        if (dir >= 0) match &= ((int)item->dir == dir);
        if (action_type >= 0) match &= ((int)item->action.type == action_type);

        if (match) {
            return item;
        }
        item = item->next;
    }
    return NULL;
}

bool tile_has_item(map_tile_t tile, int item_type, int dir, int action_type) {
    return (tile_find_item(tile, item_type, dir, action_type) != NULL);
}

bool tile_get_collide(map_tile_t tile) {
    // Collide when tiles are empty (map borders) or marked as
    // such (used for game behaviour)
    if (tile.items == NULL) {
        return true;
    } else {
        item_t *this = tile.items;
        while (this != NULL) {
            if (this->collide && !this->hidden) {
                return true;
            }
            this = this->next;
        }
    }

    return false;
}

bool map_is_empty_xy(map_t *map, int x, int y) {
    map_tile_t *tile = &map->grid[y][x];
    if (x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE) {
        return tile->items == NULL;
    } else {
        return true;
    }
}

void map_item_add_xy(map_t *map, int x, int y, item_t *new) {
    assert(x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE);
    map_tile_t *tile = &map->grid[y][x];

    if (tile->items == NULL) {
        tile->items = new;
        return;
    }

    item_t *this = tile->items;
    while (this != NULL) {
        if (this->next == NULL) {
            this->next = new;
            return;
        }
        this = this->next;
    }
}

item_t *map_has_interact_item_vec2(map_t *map, vec2 pos) {
    map_tile_t tile = map_get_tile_vec2(map, pos);
    item_t *item = tile.items;
    while (item != NULL) {
        if (item->action.type != ACTION_NONE) {
            return item;
        }
        item = item->next;
    }

    return NULL;
}

bool map_get_collide_xy(map_t *map, int x, int y) {
    return tile_get_collide(map_get_tile_xy(map, x, y));
}

bool map_get_collide_ivec2(map_t *map, ivec2 pos) {
    return tile_get_collide(map_get_tile_ivec2(map, pos));
}

bool map_get_collide_vec2(map_t *map, vec2 pos) {
    return tile_get_collide(map_get_tile_vec2(map, pos));
}

bool map_viz_get_xy(map_t *map, vec2 pos, int x, int y) {
    if (x < ((int)pos[0] - MAP_DRAW_SIZE / 2) || x >= ((int)pos[0] + MAP_DRAW_SIZE / 2) || y < ((int)pos[1] - MAP_DRAW_SIZE / 2) ||
        y >= ((int)pos[1] + MAP_DRAW_SIZE / 2)) {
        return false;
    }

    int min_x = maxi((int)pos[0] - (MAP_DRAW_SIZE / 2), 0);
    int min_y = maxi((int)pos[1] - (MAP_DRAW_SIZE / 2), 0);

    int xv = x - min_x;
    int yv = y - min_y;

    return map->viz[yv][xv];
}

bool map_viz_get_ivec2(map_t *map, vec2 pos, ivec2 tile) {
    return map_viz_get_xy(map, pos, tile[0], tile[1]);
}

void map_viz_update(map_t *map, camera_t camera) {
    const float FOV_STEP = 1.0;

    // Reset visibility
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            map->viz[y][x] = false;
        }
    }

    // Viz map boundaries
    int min_x = maxi((int)camera.pos[0] - (MAP_DRAW_SIZE / 2), 0);
    int max_x = mini((int)camera.pos[0] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);
    int min_y = maxi((int)camera.pos[1] - (MAP_DRAW_SIZE / 2), 0);
    int max_y = mini((int)camera.pos[1] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);

    // From the player position cast an FOV ray
    float a_deg = -(CAMERA_FOV / 2.0f);

    do {
        vec2 ray_dir;
        glm_vec2_rotate(camera.front, glm_rad(a_deg), ray_dir);

        vec2 ray_step;
        glm_vec2_divs(ray_dir, 3.0f, ray_step);

        vec2 ray_posf;
        glm_vec2_copy(camera.pos, ray_posf);

        ivec2 ray_posi;

        // NO ROUNDING!
        ray_posi[0] = ray_posf[0];
        ray_posi[1] = ray_posf[1];

        while (ray_posi[0] >= min_x && ray_posi[1] >= min_y && ray_posi[0] < max_x && ray_posi[1] < max_y) {
            // Viz map offset
            int xv = ray_posi[0] - min_x;
            int yv = ray_posi[1] - min_y;

            assert(xv < MAP_DRAW_SIZE);
            assert(yv < MAP_DRAW_SIZE);

            map->viz[yv][xv] = true;

            // Check for collisions
            ivec2 old_ray_posi;
            glm_ivec2_copy(ray_posi, old_ray_posi);
            glm_vec2_add(ray_posf, ray_step, ray_posf);

            // NO ROUNDING!
            ray_posi[0] = ray_posf[0];
            ray_posi[1] = ray_posf[1];

            map_tile_t tile = map_get_tile_ivec2(map, old_ray_posi);
            if (tile_has_item(tile, ITEM_WALL, DIR_SOUTH, -1)) {
                if (ray_posi[1] < old_ray_posi[1]) break;
            }
            if (tile_has_item(tile, ITEM_WALL, DIR_WEST, -1)) {
                if (ray_posi[0] > old_ray_posi[0]) break;
            }
            if (tile_has_item(tile, ITEM_WALL, DIR_NORTH, -1)) {
                if (ray_posi[1] > old_ray_posi[1]) break;
            }
            if (tile_has_item(tile, ITEM_WALL, DIR_EAST, -1)) {
                if (ray_posi[0] < old_ray_posi[0]) break;
            }
        }

        a_deg += FOV_STEP;
    } while (a_deg < (CAMERA_FOV / 2.0f));
}

void map_tile_clone_xy(map_t *map, int dst_x, int dst_y, int src_x, int src_y) {
    map_tile_t *dst_tile = &map->grid[dst_y][dst_x];
    map_tile_t *src_tile = &map->grid[src_y][src_x];
    assert(!dst_tile->is_clone);
    assert(src_tile->items != NULL);
    if (dst_tile->items != NULL) {
        item_list_free(dst_tile->items);
    }
    dst_tile->items = src_tile->items;
    dst_tile->is_clone = true;
}

map_t *map_new(void) {
    map_t *out = (map_t *)malloc(sizeof(map_t));

    out->grid = (map_tile_t **)malloc(MAP_SIZE * sizeof(map_tile_t *));

    for (int y = 0; y < MAP_SIZE; y++) {
        out->grid[y] = (map_tile_t *)malloc(MAP_SIZE * sizeof(map_tile_t));
        for (int x = 0; x < MAP_SIZE; x++) {
            // Initialize the tile
            out->grid[y][x].items = NULL;
            out->grid[y][x].is_clone = false;
        }
    }

    out->viz = (bool **)malloc(MAP_DRAW_SIZE * sizeof(bool *));
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        out->viz[y] = (bool *)malloc(MAP_DRAW_SIZE * sizeof(bool));
    }

    return out;
}

void map_free(map_t *map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // Check if tile was cloned
            map_tile_t *tile = &map->grid[y][x];
            if (!tile->is_clone) {
                item_list_free(tile->items);
            }
        }
        free(map->grid[y]);
    }
    free(map->grid);

    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        free(map->viz[y]);
    }
    free(map->viz);

    free(map);
}
