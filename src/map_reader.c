#include "map_reader.h"

minigl_matgroup_t mat_wall = {.size = 2, .color = {160, 128}};
minigl_matgroup_t mat_shutter = {.size = 3, .color = {160, 128, 96}};

static void add_room(map_t* map, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            item_t* item = item_new();
            item->type = ITEM_FLOOR;
            item->obj_id = OBJ_ID_FLOOR;
            if (((x + y) % 2) == 0) {
                item_set_color(item, 16);
            } else {
                item_set_color(item, 24);
            }
            map_item_add_xy(map, pos_x + x, pos_y + y, item);
        }
    }
}

static void add_barrier(map_t* map, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        map_item_add_xy(map, pos_x, pos_y + y, item_new_color(OBJ_ID_BARRIER, 64, DIR_WEST, true));
        map_item_add_xy(map, pos_x + width - 1, pos_y + y, item_new_color(OBJ_ID_BARRIER, 64, DIR_EAST, true));
    }

    for (int x = 0; x < width; x++) {
        map_item_add_xy(map, pos_x + x, pos_y, item_new_color(OBJ_ID_BARRIER, 64, DIR_NORTH, true));
        map_item_add_xy(map, pos_x + x, pos_y + height - 1, item_new_color(OBJ_ID_BARRIER, 64, DIR_SOUTH, true));
    }
}

static void add_door(map_t* map, int x, int y, dir_t dir, int pin) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->obj_id = OBJ_ID_SHUTTER_CLOSED;
    item->draw_mode = DRAW_MODE_MATERIAL;
    item->matgroup = &mat_shutter;
    item->dir = dir;
    item->action.type = ACTION_KEYPAD;
    item->action.arg = pin;
    map_item_add_xy(map, x, y, item);
}

static void add_note(map_t* map, int x, int y, int id) {
    item_t* item = item_new_tex(OBJ_ID_NOTE, TEX_ID_NOTE, DIR_NORTH, true);
    item->effects = EFFECT_SPIN;
    item->action.type = ACTION_NOTE;
    item->action.arg = id;
    map_item_add_xy(map, x, y, item);
}

static void add_wall(map_t* map, int x, int y, dir_t dir) {
    item_t* item = item_new();
    item->collide = true;
    item->type = ITEM_WALL;
    item->obj_id = OBJ_ID_WALL;
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

static void add_statue_with_base(map_t* map, int x, int y, dir_t dir, tex_mdbb_id_t tex_id) {
    map_item_add_xy(map, x, y, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));
    map_item_add_xy(map, x, y, item_new_mdbb(tex_id, DIR_SOUTH, true));
}

static void add_inspect_tex(map_t* map, int x, int y, tex_id_t tex_id) {
    item_t* item = item_new();
    item->hidden = true;
    item->action.type = ACTION_INSPECT;
    item->action.arg = tex_id;
    map_item_add_xy(map, x, y, item);
}

void map_gen(map_t* map) {
    //---------------------------------------------------------------------------
    // Outside
    //---------------------------------------------------------------------------
    {
        const int X = 36;
        const int Y = 61;
        const int WIDTH = 8;
        const int HEIGHT = 2;

        // Entrance(s)
        add_room(map, X + 2, Y - 1, 3, 1);

        // This section is normally unreachable
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
        map_item_add_xy(map, X + 4, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
        map_item_add_xy(map, X + 6, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                item_t* item = item_new_color(OBJ_ID_FLOOR, 64, DIR_NORTH, true);
                item->type = ITEM_FLOOR;
                map_item_add_xy(map, X + x, Y + y, item);
            }
        }
    }

    //---------------------------------------------------------------------------
    // Atrium
    //---------------------------------------------------------------------------

    // Atrium
    {
        const int X = 35;
        const int Y = 49;
        const int WIDTH = 9;
        const int HEIGHT = 11;

        // Entrance shutter
        add_room(map, X, Y, WIDTH, HEIGHT);
        add_door(map, 38, 60, DIR_NORTH, 1234);
        map_tile_clone_xy(map, 39, 60, 38, 60);
        map_tile_clone_xy(map, 40, 60, 38, 60);

        // Columns
        for (int i = 0; i < 3; i++) {
            map_item_add_xy(map, X + 2, Y + 2 + i * 3, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
            map_item_add_xy(map, X + WIDTH - 3, Y + 2 + i * 3, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
        }

        // Statue
        add_statue_with_base(map, X + 4, Y + 4, DIR_SOUTH, TEX_ID_VENUS);

        // FIXME: Move to the other room
        add_room(map, X + 3, Y - 1, 3, 1);
    }

    // Man bathroom
    {
        const int X = 45;
        const int Y = 57;
        const int WIDTH = 6;
        const int HEIGHT = 3;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X - 1, Y, 1, 1);

        // Sign
        map_item_add_xy(map, X - 2, Y + 1, item_new_tex(OBJ_ID_SIGN_SIDE_SMALL, TEX_ID_SIGN_WC_MAN, DIR_EAST, false));

        // Barrier
        map_item_add_xy(map, X - 1, Y, item_new_tex(OBJ_ID_WETFLOOR, TEX_ID_WETFLOOR, DIR_WEST, true));
    }

    // Women bathroom
    {
        const int X = 45;
        const int Y = 53;
        const int WIDTH = 6;
        const int HEIGHT = 3;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X - 1, Y + HEIGHT - 1, 1, 1);

        // Sign
        map_item_add_xy(map, X - 2, Y + HEIGHT, item_new_tex(OBJ_ID_SIGN_SIDE_SMALL, TEX_ID_SIGN_WC_WOMEN, DIR_EAST, false));

        // Stalls
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH, true));
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH, true));
        map_item_add_xy(map, X + 1, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH, true));
        map_item_add_xy(map, X + 1, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_EAST, true));

        // Sinks
        map_item_add_xy(map, X + 3, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH, true));
        map_item_add_xy(map, X + 4, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH, true));
        map_item_add_xy(map, X + 5, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH, true));
    }

    //---------------------------------------------------------------------------
    // West wing
    //---------------------------------------------------------------------------

    /*
    {
        const int X = 18;
        const int Y = 32;

        // Entrance
        add_room(map, X + 14, Y + 22, 3, 2);

        add_room(map, X + 0, Y + 0, 6, 8);
        add_room(map, X + 0, Y + 9, 11, 5);
        add_room(map, X + 7, Y + 0, 4, 10);
        add_room(map, X + 10, Y + 0, 2, 7);
        add_room(map, X + 12, Y + 0, 4, 14);
        add_room(map, X + 0, Y + 15, 5, 13);
        add_room(map, X + 4, Y + 15, 2, 7);
        add_room(map, X + 6, Y + 15, 10, 13);
        add_room(map, X + 13, Y + 14, 2, 1);
        add_room(map, X + 2, Y + 8, 2, 1);
    }

    // Four rooms
    {
        const int X = 18;
        const int Y = 46;

        // Entrance
        add_room(map, X + 14, Y + 7, 3, 2);

        // Barrier
        add_barrier(map, X + 12, Y + 6, 2, 0);
        add_barrier(map, X + 10, Y + 2, 0, 2);

        // Corridors
        add_room(map, X + 6, Y + 2, 4, 2);
        add_room(map, X + 2, Y + 6, 2, 4);
        add_room(map, X + 6, Y + 12, 4, 2);
        add_room(map, X + 12, Y + 6, 2, 4);

        // North-west
        add_room(map, X, Y + 0, 6, 6);

        // South-west
        add_room(map, X, Y + 10, 6, 6);

        // North-east
        add_room(map, X + 10, Y + 0, 6, 6);

        // South-east
        add_room(map, X + 10, Y + 10, 6, 6);

        // Exit
        add_room(map, X + 7, Y - 2, 2, 4);
    }

    // Big-room
    {
        const int X = 20;
        const int Y = 32;

        // Main room
        add_room(map, X + 0, Y, 5, 3);
        add_room(map, X + 6, Y, 2, 3);
        add_room(map, X, Y + 3, 8, 2);
        add_room(map, X + 1, Y + 5, 1, 1);  // e

        add_room(map, X + 0, Y + 6, 2, 3);
        add_room(map, X + 3, Y + 6, 8, 2);
        add_room(map, X + 3, Y + 8, 1, 1);

        add_room(map, X + 8, Y + 8, 2, 1);
        add_room(map, X + 0, Y + 9, 4, 3);
        add_room(map, X + 5, Y + 9, 6, 3);

        // Employee only sign
        map_item_add_xy(map, X + 3, Y, item_new_tex(OBJ_ID_SIGN_SIDE_SMALL, TEX_ID_SIGN_EMPLOYEEONLY, DIR_NORTH, false));
    }*/

    //---------------------------------------------------------------------------
    // Central wing
    //---------------------------------------------------------------------------

    // Room 1
    {
        const int X = 35;
        const int Y = 41;

        add_room(map, X, Y, 9, 3);
        add_room(map, X, Y + 3, 2, 1);
        add_room(map, X + 7, Y + 3, 2, 1);
        add_room(map, X, Y + 4, 9, 3);

        // Pictures
        map_item_add_xy(map, X + 3, Y + 4, item_new_tex(OBJ_ID_PICTURE_RECT, TEX_ID_PICTURE_0, DIR_NORTH, false));
        map_item_add_xy(map, X + 5, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_1, DIR_NORTH, false));

        map_item_add_xy(map, X + 3, Y + 2, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_C, TEX_ID_PICTURE_2, DIR_SOUTH, false));
        map_item_add_xy(map, X + 5, Y + 2, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_3, DIR_SOUTH, false));
    }

    // Room 2
    {
        const int X = 35;
        const int Y = 33;
        const int WIDTH = 9;
        const int HEIGHT = 7;

        add_room(map, X, Y, 9, 3);
        add_room(map, X, Y + 3, 2, 1);
        add_room(map, X + 7, Y + 3, 2, 1);
        add_room(map, X, Y + 4, 9, 3);

        // Entrance(s)
        add_room(map, X + 2, Y + HEIGHT, 2, 1);
        add_room(map, X + WIDTH - 4, Y + HEIGHT, 2, 1);

        // Pictures
        map_item_add_xy(map, X + 3, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_0, TEX_ID_PICTURE_4, DIR_NORTH, false));
        map_item_add_xy(map, X + 3, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_1, TEX_ID_PICTURE_5, DIR_NORTH, false));
        map_item_add_xy(map, X + 3, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_2, TEX_ID_PICTURE_6, DIR_NORTH, false));
        map_item_add_xy(map, X + 3, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_3, TEX_ID_PICTURE_7, DIR_NORTH, false));
        map_item_add_xy(map, X + 5, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_8, DIR_NORTH, false));

        map_item_add_xy(map, X + 3, Y + 2, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_C, TEX_ID_PICTURE_0, DIR_SOUTH, false));
        map_item_add_xy(map, X + 5, Y + 2, item_new_tex(OBJ_ID_PICTURE_RECT, TEX_ID_PICTURE_1, DIR_SOUTH, false));
    }

    // Gallery
    {
        const int X = 45;
        const int Y = 33;
        const int WIDTH = 6;
        const int HEIGHT = 15;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X - 1, Y + 2, 1, 2);
        add_room(map, X - 1, Y + 11, 1, 2);

        // Benches
        map_item_add_xy(map, X + 2, Y + 3, item_new_color(OBJ_ID_BENCH, 100, DIR_WEST, true));
        map_item_add_xy(map, X + 2, Y + 7, item_new_color(OBJ_ID_BENCH, 100, DIR_WEST, true));
        map_item_add_xy(map, X + 2, Y + 11, item_new_color(OBJ_ID_BENCH, 100, DIR_WEST, true));

        // Pictures
        map_item_add_xy(map, X + 5, Y + 2, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_0, TEX_ID_PICTURE_0, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 2, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_3, TEX_ID_PICTURE_1, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 4, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_2, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 6, item_new_tex(OBJ_ID_PICTURE_RECT, TEX_ID_PICTURE_3, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 8, item_new_tex(OBJ_ID_PICTURE_RECT, TEX_ID_PICTURE_4, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 10, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_5, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 12, item_new_tex(OBJ_ID_PICTURE_SQUARE_SMALL_C, TEX_ID_PICTURE_6, DIR_EAST, false));
    }

    //---------------------------------------------------------------------------
    // North wing
    //---------------------------------------------------------------------------

    // Corridor
    {
        const int X = 37;
        const int Y = 19;
        const int WIDTH = 5;
        const int HEIGHT = 13;

        add_room(map, X, Y, WIDTH, HEIGHT);

        map_item_add_xy(map, X, Y + 6, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_1, DIR_WEST, false));
        map_item_add_xy(map, X + WIDTH - 1, Y + 6, item_new_tex(OBJ_ID_PICTURE_SQUARE, TEX_ID_PICTURE_3, DIR_EAST, false));

        // Entrance(s)
        add_room(map, X + 1, Y + HEIGHT, 3, 1);
    }

    // Room 1
    {
        const int X = 30;
        const int Y = 19;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X + WIDTH, Y + 2, 1, 2);

        // Employee only sign
        map_item_add_xy(map, X, Y, item_new_tex(OBJ_ID_SIGN_SIDE_SMALL, TEX_ID_SIGN_EMPLOYEEONLY, DIR_WEST, false));
    }

    // Room 2
    {
        const int X = 30;
        const int Y = 26;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X + WIDTH, Y + 2, 1, 2);
        add_room(map, X + 2, Y - 1, 2, 1);
    }

    // Room 3
    {
        const int X = 43;
        const int Y = 19;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X - 1, Y + 2, 1, 2);
    }

    // Room 4
    {
        const int X = 43;
        const int Y = 26;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X - 1, Y + 2, 1, 2);
        add_room(map, X + 2, Y - 1, 2, 1);
    }

    //---------------------------------------------------------------------------
    // Big room
    //---------------------------------------------------------------------------
    {
        const int X = 30;
        const int Y = 1;

        const int WIDTH = 20;
        const int HEIGHT = 17;

        // Entrance(s)
        add_room(map, X + 8, Y + HEIGHT, 3, 1);

        // Floor
        add_room(map, X, Y, WIDTH, HEIGHT);

        /*// Columns*/
        /*for (int i = 0; i < 2; i++) {*/
        /*    for (int i = 0; i < 2; i++) {*/
        /*        map_item_add_xy(map, X + 4, Y + 4 + i * 3, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));*/
        /*    }*/
        /*}*/
    }

    {
        const int X = 19;
        const int Y = 41;

        add_room(map, X + 15, Y + 12, 1, 3);

        add_room(map, X + 0, Y + 0, 5, 7);
        add_room(map, X + 0, Y + 8, 10, 3);
        add_room(map, X + 6, Y + 0, 4, 8);
        add_room(map, X + 10, Y + 0, 2, 7);
        add_room(map, X + 11, Y + 0, 4, 11);
        add_room(map, X + 0, Y + 12, 5, 9);
        add_room(map, X + 5, Y + 12, 2, 4);
        add_room(map, X + 6, Y + 12, 9, 9);
        add_room(map, X + 1, Y + 6, 2, 2);
        add_room(map, X + 12, Y + 10, 2, 2);
    }

    //---------------------------------------------------------------------------
    // Automatically add walls
    //---------------------------------------------------------------------------

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (tile_has_item(map->grid[y][x], ITEM_FLOOR, -1, -1)) {
                if (!tile_has_item(map->grid[y - 1][x], ITEM_FLOOR, -1, -1)) {
                    add_wall(map, x, y - 1, DIR_SOUTH);
                }
                if (!tile_has_item(map->grid[y][x + 1], ITEM_FLOOR, -1, -1)) {
                    add_wall(map, x + 1, y, DIR_WEST);
                }
                if (map_is_empty_xy(map, x + 1, y)) {
                    add_wall(map, x, y, DIR_EAST);
                }
                if (!tile_has_item(map->grid[y + 1][x], ITEM_FLOOR, -1, -1)) {
                    add_wall(map, x, y + 1, DIR_NORTH);
                }
                if (!tile_has_item(map->grid[y][x - 1], ITEM_FLOOR, -1, -1)) {
                    add_wall(map, x - 1, y, DIR_EAST);
                }
            }
        }
    }
}
