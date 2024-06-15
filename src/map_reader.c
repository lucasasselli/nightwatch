#include "map_reader.h"

static void add_room(map_t map, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            item_t* item = item_new();
            item->type = ITEM_FLOOR;
            item->obj_id = OBJ_FLOOR;
            if (((x + y) % 2) == 0) {
                item_set_color(item, 16);
            } else {
                item_set_color(item, 24);
            }
            map_item_add_xy(map, pos_x + x, pos_y + y, item);
        }
    }
}

static void add_door(map_t map, int x, int y, dir_t dir, int pin) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->obj_id = OBJ_WALL;
    item->tex_id = TEX_FENCE_CLOSED;
    item->tex_mode = TEX_MODE_IMAGE;
    item->dir = dir;
    item->action.type = ACTION_KEYPAD;
    item->action.arg = pin;
    map_item_add_xy(map, x, y, item);
}

static void add_note(map_t map, int x, int y, int id) {
    item_t* item = item_new_tex(OBJ_NOTE, TEX_NOTE, DIR_NORTH);
    item->effects = EFFECT_SPIN;
    item->action.type = ACTION_NOTE;
    item->action.arg = id;
    map_item_add_xy(map, x, y, item);
}

static void add_wall(map_t map, int x, int y, dir_t dir) {
    item_t* item = item_new();
    item->type = ITEM_WALL;
    item->obj_id = OBJ_WALL;
    item->dir = dir;

    switch (dir) {
        case DIR_NORTH:
            item_set_color(item, 160);
            break;
        case DIR_EAST:
            item_set_color(item, 128);
            break;
        case DIR_SOUTH:
            item_set_color(item, 160);
            break;
        case DIR_WEST:
            item_set_color(item, 128);
            break;
    }
    map_item_add_xy(map, x, y, item);
}

void map_read(map_t map) {
    // TODO: Implement read from file?

    //---------------------------------------------------------------------------
    // Entrance
    //---------------------------------------------------------------------------

    // This section is normally unreachable
    map_item_add_xy(map, 28, 62, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
    map_item_add_xy(map, 30, 62, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
    map_item_add_xy(map, 32, 62, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
    map_item_add_xy(map, 34, 62, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));

    add_door(map, 30, 60, DIR_NORTH, 1234);
    add_door(map, 31, 60, DIR_NORTH, 1234);
    add_door(map, 32, 60, DIR_NORTH, 1234);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 8; x++) {
            // map_item_add_xy(map, 28 + x, 60 + y, item_new_color(OBJ_FLOOR, 100, DIR_NORTH));
        }
    }

    //---------------------------------------------------------------------------
    // Section 1
    //---------------------------------------------------------------------------

    {
        const int X = 27;
        const int Y = 49;
        const int WIDTH = 9;
        const int HEIGHT = 11;

        // Entrance
        add_room(map, X, Y, WIDTH, HEIGHT);

        map_item_add_xy(map, X + 2, Y + 2 + 0, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y + 2 + 3, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y + 2 + 6, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 0, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 3, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 6, item_new_color(OBJ_COLUMN, 100, DIR_NORTH));

        add_note(map, X + 4, Y + 7, 0);

        // Mens Bathroom
        // Unreachable
        add_room(map, 36, 58, 1, 1);
        add_room(map, 37, 58, 6, 3);

        map_item_add_xy(map, 36, 58, item_new_tex(OBJ_WETFLOOR, TEX_WETFLOOR, DIR_WEST));

        // Womens Bathroom
        add_room(map, 36, 56, 1, 1);
        add_room(map, 37, 54, 6, 3);

        // Section 1 door
        add_room(map, X + 3, Y - 1, 3, 1);
        add_door(map, X + 3 + 0, Y - 1, DIR_SOUTH, 1234);
        add_door(map, X + 3 + 1, Y - 1, DIR_SOUTH, 1234);
        add_door(map, X + 3 + 2, Y - 1, DIR_SOUTH, 1234);
    }

    //---------------------------------------------------------------------------
    // Section 2
    //---------------------------------------------------------------------------

    {
        const int X = 26;
        const int Y = 41;
        const int WIDTH = 11;
        const int HEIGHT = 7;

        // Room 1
        add_room(map, X, Y, WIDTH, HEIGHT);

        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mdbb(TEX_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 3, Y + 2 + 0, item_new_mdbb(TEX_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 6, Y + 2 + 0, item_new_mdbb(TEX_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 2, item_new_mdbb(TEX_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 3, Y + 2 + 2, item_new_mdbb(TEX_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 6, Y + 2 + 2, item_new_mdbb(TEX_VENUS, DIR_NORTH));
    }

    /*

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

    */

    // Add walls
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (tile_has_item(map[y][x], ITEM_FLOOR, -1, -1)) {
                if (map_is_empty_xy(map, x, y - 1)) {
                    add_wall(map, x, y, DIR_NORTH);
                }
                if (map_is_empty_xy(map, x + 1, y)) {
                    add_wall(map, x, y, DIR_EAST);
                }
                if (map_is_empty_xy(map, x, y + 1)) {
                    add_wall(map, x, y, DIR_SOUTH);
                }
                if (map_is_empty_xy(map, x - 1, y)) {
                    add_wall(map, x, y, DIR_WEST);
                }
            }
        }
    }
}
