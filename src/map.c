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
            if (this->collide) {
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

static void map_viz_set_xy(map_t *map, ivec2 pos, int x, int y, bool viz) {
    int min_x = maxi((int)pos[0] - (MAP_DRAW_SIZE / 2), 0);
    int min_y = maxi((int)pos[1] - (MAP_DRAW_SIZE / 2), 0);

    int xv = x - min_x;
    int yv = y - min_y;

    assert(xv < MAP_DRAW_SIZE);
    assert(yv < MAP_DRAW_SIZE);

    map->viz[yv][xv] = viz;
}

static void viz_dda_raycast(map_t *map, ivec2 ppos, ivec2 tpos) {
    // Digital Differential Analysis
    // NOTE: https://lodev.org/cgtutor/raycasting.html
    float rayDirX = tpos[0] - ppos[0];
    float rayDirY = tpos[1] - ppos[1];

    float deltaDistX = (rayDirX == 0) ? 1e30f : fabsf(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : fabsf(1.0f / rayDirY);

    float posX = ppos[0];
    float posY = ppos[1];

    // length of ray from current position to next x or y-side
    float sideDistX;
    float sideDistY;

    // what direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    ivec2 i;
    glm_ivec2_copy(ppos, i);

    if (rayDirX < 0) {
        stepX = -1.0f;
        sideDistX = (posX - i[0]) * deltaDistX;
    } else {
        stepX = 1.0f;
        sideDistX = (i[0] + 1.0f - posX) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1.0f;
        sideDistY = (posY - i[1]) * deltaDistY;
    } else {
        stepY = 1.0f;
        sideDistY = (i[1] + 1.0f - posY) * deltaDistY;
    }

    bool side;
    bool hit = false;

    int min_x = maxi((int)ppos[0] - (MAP_DRAW_SIZE / 2), 0);
    int max_x = mini((int)ppos[0] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);
    int min_y = maxi((int)ppos[1] - (MAP_DRAW_SIZE / 2), 0);
    int max_y = mini((int)ppos[1] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);

    do {
        // Check if ray has hit a wall
        int x = i[0];
        int y = i[1];

        map_tile_t *tile = &map->grid[y][x];

        map_viz_set_xy(map, ppos, x, y, true);

        if (hit) break;

        // jump to next i square, either in x-direction, or in y-direction
        if (sideDistX < sideDistY) {
            // East-west
            sideDistX += deltaDistX;
            i[0] += stepX;
            side = 0;
        } else {
            // North-south
            sideDistY += deltaDistY;
            i[1] += stepY;
            side = 1;
        }

        if (side) {
            // North-south
            if (stepY > 0) {
                hit |= tile_has_item(*tile, ITEM_WALL, DIR_SOUTH, -1);
            } else {
                hit |= tile_has_item(*tile, ITEM_WALL, DIR_NORTH, -1);
            }
        } else {
            // East-west
            if (stepX > 0) {
                hit |= tile_has_item(*tile, ITEM_WALL, DIR_EAST, -1);
            } else {
                hit |= tile_has_item(*tile, ITEM_WALL, DIR_WEST, -1);
            }
        }

    } while (!hit && i[0] >= min_x && i[1] >= min_y && i[0] < max_x && i[1] < max_y);
}

static void viz_to_check(map_t *map, camera_t camera, int x, int y) {
    const float FOV_HEADROOM_ANGLE = 12.0f;
    // NOTE: Use the cross product of the position-tile vector and the
    // camera direction to check if the tile is within the FOV.
    vec2 dir;
    tile_dir((ivec2){x, y}, camera.pos, dir);

    // Check agains FOV angle
    float a = glm_vec2_dot(dir, camera.front);
    if (a >= cosf(glm_rad(CAMERA_FOV / 2.0f + FOV_HEADROOM_ANGLE))) {
        // Tile is in FOV, is there anything in between?
        viz_dda_raycast(map, (ivec2){roundf(camera.pos[0]), roundf(camera.pos[1])}, (ivec2){x, y});
    }
}

void map_viz_update(map_t *map, camera_t camera) {
    // Reset visibility
    for (int y = 0; y < MAP_DRAW_SIZE; y++) {
        for (int x = 0; x < MAP_DRAW_SIZE; x++) {
            map->viz[y][x] = true;
        }
    }

    // int viz_off_x = maxi((int)ppos[0] - (MAP_DRAW_SIZE / 2), 0);
    // int viz_off_y = maxi((int)ppos[1] - (MAP_DRAW_SIZE / 2), 0);

    for (int i = 0; i < MAP_SIZE; i++) {
        viz_to_check(map, camera, 0, i);
        viz_to_check(map, camera, MAP_SIZE - 1, i);
        viz_to_check(map, camera, i, 0);
        viz_to_check(map, camera, i, MAP_SIZE - 1);
    }
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
            if (tile->is_clone) {
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
