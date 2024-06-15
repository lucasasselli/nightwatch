#include "map_reader.h"

static void add_room(map_t map, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            map_item_add_xy(map, pos_x + x, pos_y + y, MAP_ITEM_STATIC_INIT(ITEM_FLOOR, DIR_ANY));
        }
    }
}

static void add_note(map_t map, int x, int y, int id) {
    map_item_t item = {.type = ITEM_NOTE, .dir = DIR_ANY, .hidden = false, .action = true, .arg = id};
    map_item_add_xy(map, x, y, item);
}

static void add_door(map_t map, int x, int y, map_item_dir_t dir, int pin) {
    map_item_t item = {.type = ITEM_DOOR, .dir = dir, .hidden = false, .action = true, .arg = pin};
    map_item_add_xy(map, x, y, item);
}

void map_read(map_t map) {
    // Initialize

    // TODO: Implement read from file

    // Mens Bathroom
    add_room(map, 36, 34, 1, 1);
    add_room(map, 37, 34, 6, 3);

    // Womens Bathroom
    add_room(map, 36, 32, 1, 1);
    add_room(map, 37, 30, 6, 3);

    // Entrance
    add_room(map, 27, 26, 9, 11);

    map_item_add_xy(map, 29, 28, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));
    map_item_add_xy(map, 29, 31, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));
    map_item_add_xy(map, 29, 34, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));
    map_item_add_xy(map, 33, 28, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));
    map_item_add_xy(map, 33, 31, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));
    map_item_add_xy(map, 33, 34, MAP_ITEM_STATIC_INIT(ITEM_COLUMN, DIR_ANY));

    map_item_add_xy(map, 31, 31, MAP_ITEM_STATIC_INIT(ITEM_BASE, DIR_SOUTH));

    add_note(map, 31, 32, 0);

    // Room 1
    add_room(map, 30, 24, 3, 2);

    add_room(map, 28, 23, 6, 1);
    add_room(map, 28, 22, 5, 1);
    add_room(map, 28, 21, 6, 1);
    add_room(map, 28, 20, 5, 1);
    add_room(map, 28, 19, 6, 1);

    map_item_add_xy(map, 29, 22, MAP_ITEM_STATIC_INIT(ITEM_WETFLOOR, DIR_WEST));

    map_item_add_xy(map, 33, 23, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST));
    map_item_add_xy(map, 33, 21, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST));
    map_item_add_xy(map, 33, 19, MAP_ITEM_STATIC_INIT(ITEM_STATUE, DIR_WEST));

    // Gallery corridor
    add_room(map, 30, 18, 2, 2);

    add_room(map, 28, 16, 6, 2);  // South
    add_room(map, 28, 10, 2, 6);  // West
    add_room(map, 32, 10, 2, 6);  // East
    add_room(map, 28, 8, 6, 2);   // South

    // Gallery
    add_room(map, 30, 6, 2, 2);

    add_room(map, 30, 0, 10, 6);
    add_room(map, 29, 2, 1, 2);
    add_room(map, 19, 0, 10, 4);
    add_room(map, 18, 0, 1, 3);
    add_room(map, 16, 0, 2, 4);

    // Room 3
    add_room(map, 26, 20, 2, 3);

    add_room(map, 18, 18, 8, 8);
    add_room(map, 9, 18, 8, 8);

    add_room(map, 17, 19, 1, 2);
    add_room(map, 17, 23, 1, 2);

    // Employee area
    add_room(map, 5, 23, 4, 1);  // Corridor

    add_door(map, 9, 23, DIR_WEST, 1234);

    add_room(map, 1, 22, 4, 4);  // Room

    // Add walls
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (map_tile_has_item(map[y][x], ITEM_FLOOR, DIR_ANY)) {
                if (map_tile_is_empty_xy(map, x, y - 1)) {
                    map_item_add_xy(map, x, y, (map_item_t){ITEM_WALL, DIR_NORTH});
                }
                if (map_tile_is_empty_xy(map, x + 1, y)) {
                    map_item_add_xy(map, x, y, (map_item_t){ITEM_WALL, DIR_EAST});
                }
                if (map_tile_is_empty_xy(map, x, y + 1)) {
                    map_item_add_xy(map, x, y, (map_item_t){ITEM_WALL, DIR_SOUTH});
                }
                if (map_tile_is_empty_xy(map, x - 1, y)) {
                    map_item_add_xy(map, x, y, (map_item_t){ITEM_WALL, DIR_WEST});
                }
            }
        }
    }
}
