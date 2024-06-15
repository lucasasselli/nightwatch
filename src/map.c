#include "map.h"

#include <assert.h>

#include "constants.h"
#include "utils.h"

map_tile_t map_get_tile_xy(map_t map, int x, int y) {
    assert(x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE);

    return map[y][x];
}

map_tile_t map_get_tile_ivec2(map_t map, ivec2 pos) {
    return map_get_tile_xy(map, pos[0], pos[1]);
}

map_tile_t map_get_tile_vec2(map_t map, vec2 pos) {
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
    return (tile.items == NULL || tile.collide);
}

bool map_is_empty_xy(map_t map, int x, int y) {
    map_tile_t *tile = &map[y][x];
    if (x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE) {
        return tile->items == NULL;
    } else {
        return true;
    }
}

void map_item_add_xy(map_t map, int x, int y, item_t *new) {
    assert(x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE);
    map_tile_t *tile = &map[y][x];

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

bool map_get_collide_xy(map_t map, int x, int y) {
    return tile_get_collide(map_get_tile_xy(map, x, y));
}

bool map_get_collide_ivec2(map_t map, ivec2 pos) {
    return tile_get_collide(map_get_tile_ivec2(map, pos));
}

void map_set_collide_xy(map_t map, int x, int y, bool collide) {
    map[y][x].collide = collide;
}

static void viz_dda_raycast(map_t map, ivec2 ppos, ivec2 tpos) {
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

    do {
        // Check if ray has hit a wall
        map_tile_t *tile = &map[i[1]][i[0]];
        tile->visible = true;

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

    } while (!hit && i[0] >= 0 && i[1] >= 0 && i[0] < MAP_SIZE && i[1] < MAP_SIZE);
}

static void viz_to_check(map_t map, camera_t camera, int x, int y) {
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

// FIXME: tiles very close to the player might disappear
void map_viz_update(map_t map, camera_t camera) {
    int min_x = maxi((int)camera.pos[0] - (MAP_DRAW_SIZE / 2), 0);
    int max_x = mini((int)camera.pos[0] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);
    int min_y = maxi((int)camera.pos[1] - (MAP_DRAW_SIZE / 2), 0);
    int max_y = mini((int)camera.pos[1] + (MAP_DRAW_SIZE / 2), MAP_SIZE - 1);

    for (int y = min_y; y < max_y; y++) {
        for (int x = min_x; x < max_x; x++) {
            map[y][x].visible = false;  // FIXME: Visibility for every tile is a little bit too much
        }
    }

    for (int i = 0; i < MAP_SIZE; i++) {
        viz_to_check(map, camera, min_x, i);
        viz_to_check(map, camera, max_x, i);
        viz_to_check(map, camera, i, min_y);
        viz_to_check(map, camera, i, max_y);
    }
}

bool map_viz_xy(map_t map, vec2 pos, int x, int y) {
    if (x < ((int)pos[0] - MAP_DRAW_SIZE / 2) || x >= ((int)pos[0] + MAP_DRAW_SIZE / 2) || y < ((int)pos[1] - MAP_DRAW_SIZE / 2) ||
        y >= ((int)pos[1] + MAP_DRAW_SIZE / 2)) {
        return false;
    }
    return map_get_tile_xy(map, x, y).visible;
}

bool map_viz_ivec2(map_t map, vec2 pos, ivec2 tile) {
    return map_viz_xy(map, pos, tile[0], tile[1]);
}

void map_init(map_t map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // Initialize the tile
            map[y][x].items = NULL;
        }
    }
}
