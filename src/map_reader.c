#include "map_reader.h"

#include <assert.h>

static void map_tile_item_add(map_t map, map_item_t item, int x, int y) {
    assert(x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE);
    map_tile_t *tile = &map[y][x];
    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
}

static bool map_tile_is_empty(map_t map, int x, int y) {
    map_tile_t *tile = &map[y][x];
    if (x >= 0 && y >= 0 && x < MAP_SIZE && y < MAP_SIZE) {
        return tile->item_cnt == 0;
    } else {
        return true;
    }
}

void map_square(map_t map, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            map_tile_item_add(map, (map_item_t){ITEM_FLOOR, DIR_ANY, false, 0, false}, pos_x + x, pos_y + y);
        }
    }
}

void map_read(map_t map) {
    // Initialize

    // TODO: Implement read from file

    // Mens Bathroom
    map_square(map, 36, 34, 1, 1);
    map_square(map, 37, 34, 6, 3);

    // Womens Bathroom
    map_square(map, 36, 32, 1, 1);
    map_square(map, 37, 30, 6, 3);

    // Entrance
    map_square(map, 27, 26, 9, 11);

    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 29, 28);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 29, 31);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 29, 34);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 33, 28);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 33, 31);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY), 33, 34);

    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_BASE, DIR_SOUTH), 31, 31);
    map_tile_item_add(map, MAP_ITEM_ACTION_INIT(ITEM_NOTE, DIR_ANY, 0), 31, 32);

    // Room 1
    map_square(map, 30, 24, 3, 2);

    map_square(map, 28, 23, 6, 1);
    map_square(map, 28, 22, 5, 1);
    map_square(map, 28, 21, 6, 1);
    map_square(map, 28, 20, 5, 1);
    map_square(map, 28, 19, 6, 1);

    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_WETFLOOR, DIR_WEST), 29, 22);

    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST), 33, 23);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST), 33, 21);
    map_tile_item_add(map, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST), 33, 19);

    // Gallery corridor
    map_square(map, 30, 18, 2, 2);

    map_square(map, 28, 16, 6, 2);  // South
    map_square(map, 28, 10, 2, 6);  // West
    map_square(map, 32, 10, 2, 6);  // East
    map_square(map, 28, 8, 6, 2);   // South

    // Gallery
    map_square(map, 30, 6, 2, 2);

    map_square(map, 30, 0, 10, 6);
    map_square(map, 29, 2, 1, 2);
    map_square(map, 19, 0, 10, 4);
    map_square(map, 18, 0, 1, 3);
    map_square(map, 16, 0, 2, 4);

    // Room 3
    map_square(map, 26, 20, 2, 3);

    map_square(map, 18, 18, 8, 8);
    map_square(map, 9, 18, 8, 8);

    map_square(map, 17, 19, 1, 2);
    map_square(map, 17, 23, 1, 2);

    // Employee area
    map_square(map, 5, 23, 4, 1);  // Corridor
    map_tile_item_add(map, MAP_ITEM_ACTION_INIT(ITEM_DOOR, DIR_WEST, 0), 9, 23);

    map_square(map, 1, 22, 4, 4);  // Room

    // Add walls
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (map_tile_has_item(map[y][x], ITEM_FLOOR, DIR_ANY)) {
                if (map_tile_is_empty(map, x, y - 1)) {
                    map_tile_item_add(map, (map_item_t){ITEM_WALL, DIR_NORTH}, x, y);
                }
                if (map_tile_is_empty(map, x + 1, y)) {
                    map_tile_item_add(map, (map_item_t){ITEM_WALL, DIR_EAST}, x, y);
                }
                if (map_tile_is_empty(map, x, y + 1)) {
                    map_tile_item_add(map, (map_item_t){ITEM_WALL, DIR_SOUTH}, x, y);
                }
                if (map_tile_is_empty(map, x - 1, y)) {
                    map_tile_item_add(map, (map_item_t){ITEM_WALL, DIR_WEST}, x, y);
                }
            }
        }
    }
}
