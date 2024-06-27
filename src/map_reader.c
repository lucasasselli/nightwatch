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

void map_read(map_t* map) {
    // TODO: Implement read from file?

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    // Outside
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    {
        const int X = 28;
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
    //---------------------------------------------------------------------------
    // Level 1
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    // Atrium
    {
        const int X = 27;
        const int Y = 49;
        const int WIDTH = 9;
        const int HEIGHT = 11;

        // Entrance shutter
        add_room(map, X, Y, WIDTH, HEIGHT);
        add_door(map, 30, 60, DIR_NORTH, 1234);
        map_tile_clone_xy(map, 31, 60, 30, 60);
        map_tile_clone_xy(map, 32, 60, 30, 60);

        // Columns
        map_item_add_xy(map, X + 2, Y + 2 + 0, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + 2, Y + 2 + 0, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_7, DIR_SOUTH, false));

        map_item_add_xy(map, X + 2, Y + 2 + 3, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + 2, Y + 2 + 3, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_3, DIR_SOUTH, false));

        map_item_add_xy(map, X + 2, Y + 2 + 6, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + 2, Y + 2 + 6, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_0, DIR_SOUTH, false));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 0, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 0, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_6, DIR_SOUTH, false));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 3, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 3, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_1, DIR_SOUTH, false));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 6, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH, true));
        // map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 6, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_4, DIR_SOUTH, false));

        // Statue
        // TODO: Replace with something else!
        map_item_add_xy(map, X + 4, Y + 4, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));
        map_item_add_xy(map, X + 4, Y + 4 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));

        // Museum history sign
        item_t* sign_hist = item_new_tex(OBJ_ID_SIGN_HUGE, TEX_ID_SIGN_MUSEUM_HISTORY, DIR_EAST, false);
        sign_hist->action.type = ACTION_INSPECT;
        sign_hist->action.arg = TEX_ID_SIGN_MUSEUM_HISTORY;

        map_item_add_xy(map, X + WIDTH - 1, Y + 2, sign_hist);
        add_note(map, X + WIDTH - 2, Y + 2, 2);

        // Level 2 shutter
        add_room(map, X + 3, Y - 1, 3, 1);
        add_door(map, X + 3 + 0, Y - 1, DIR_SOUTH, 9568);
        map_tile_clone_xy(map, X + 3 + 1, Y - 1, X + 3 + 0, Y - 1);
        map_tile_clone_xy(map, X + 3 + 2, Y - 1, X + 3 + 0, Y - 1);

        // Note(s)
        add_note(map, X + 4, Y + 7, 0);
    }

    //---------------------------------------------------------------------------
    // Man bathroom
    //---------------------------------------------------------------------------
    // NOTE: Unreachable
    {
        const int X = 37;
        const int Y = 57;
        const int WIDTH = 6;
        const int HEIGHT = 3;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X - 1, Y, 1, 1);

        // Sign
        map_item_add_xy(map, X - 2, Y + 1, item_new_tex(OBJ_ID_SIGN_SMALL_SIDE, TEX_ID_SIGN_WC_MAN, DIR_EAST, false));

        // Barrier
        map_item_add_xy(map, X - 1, Y, item_new_tex(OBJ_ID_WETFLOOR, TEX_ID_WETFLOOR, DIR_WEST, true));
    }

    //---------------------------------------------------------------------------
    // Women bathroom
    //---------------------------------------------------------------------------
    {
        const int X = 37;
        const int Y = 53;
        const int WIDTH = 6;
        const int HEIGHT = 3;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X - 1, Y + HEIGHT - 1, 1, 1);

        // Sign
        map_item_add_xy(map, X - 2, Y + HEIGHT, item_new_tex(OBJ_ID_SIGN_SMALL_SIDE, TEX_ID_SIGN_WC_WOMEN, DIR_EAST, true));

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

        // Note(s)
        add_note(map, X + 4, Y, 1);
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    // Level 2
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Room 1
    //---------------------------------------------------------------------------
    {
        const int X = 27;
        const int Y = 41;
        const int WIDTH = 9;
        const int HEIGHT = 7;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Statues - Upper row
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 7
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_tex(OBJ_ID_SIGN_BASE, TEX_ID_ROMAN_4, DIR_NORTH, false));

        map_item_add_xy(map, X + 2 + 2, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 8

        map_item_add_xy(map, X + 2 + 4, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 9
        map_item_add_xy(map, X + 2 + 4, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2 + 4, Y + 2 + 0, item_new_tex(OBJ_ID_SIGN_BASE, TEX_ID_ROMAN_1, DIR_NORTH, false));

        // Statues - Lower row
        map_item_add_xy(map, X + 2 + 2, Y + 2 + 2, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 0

        add_note(map, X + 2 + 2, Y + 2 + 4, 3);
    }

    //---------------------------------------------------------------------------
    // Room 2
    //---------------------------------------------------------------------------
    {
        const int X = 27;
        const int Y = 33;
        const int WIDTH = 9;
        const int HEIGHT = 7;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X + 2, Y + HEIGHT, 2, 1);
        add_room(map, X + WIDTH - 4, Y + HEIGHT, 2, 1);

        // Statues - Upper row
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 1
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_tex(OBJ_ID_SIGN_BASE, TEX_ID_ROMAN_2, DIR_NORTH, false));

        map_item_add_xy(map, X + 2 + 2, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 2

        map_item_add_xy(map, X + 2 + 4, Y + 2 + 0, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 3

        // Statues - Lower row
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 2, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 4

        map_item_add_xy(map, X + 2 + 2, Y + 2 + 2, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 5
        map_item_add_xy(map, X + 2 + 2, Y + 2 + 2, item_new_mdbb(TEX_ID_VENUS, DIR_NORTH, true));
        map_item_add_xy(map, X + 2 + 2, Y + 2 + 2, item_new_tex(OBJ_ID_SIGN_BASE, TEX_ID_ROMAN_3, DIR_NORTH, false));

        map_item_add_xy(map, X + 2 + 4, Y + 2 + 2, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));  // 6
    }

    //---------------------------------------------------------------------------
    // Gallery
    //---------------------------------------------------------------------------
    {
        const int X = 37;
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
        map_item_add_xy(map, X + 5, Y + 2, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_0, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 5, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_2, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 9, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_4, DIR_EAST, false));
        map_item_add_xy(map, X + 5, Y + 12, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_5, DIR_EAST, false));

        // Level 3 shutter
        add_door(map, X + 1, Y + HEIGHT, DIR_NORTH, 2509);
        add_note(map, X + 1, Y + HEIGHT - 1, 4);

        // Level 4 shutter
        add_door(map, X + 2 + 0, Y - 1, DIR_SOUTH, 9157);
        map_tile_clone_xy(map, X + 2 + 1, Y - 1, X + 2 + 0, Y - 1);
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    // Level 3
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Break room
    //---------------------------------------------------------------------------
    {
        const int X = 37;
        const int Y = 49;
        const int WIDTH = 6;
        const int HEIGHT = 3;

        add_room(map, X, Y, WIDTH, HEIGHT);

        // Entrance(s)
        add_room(map, X + 1, Y - 1, 1, 1);
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    // Level 4
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Corridor
    //---------------------------------------------------------------------------
    {
        const int X = 38;
        const int Y = 19;
        const int WIDTH = 4;
        const int HEIGHT = 13;

        add_room(map, X, Y, WIDTH, HEIGHT);

        map_item_add_xy(map, X, Y + 6, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_1, DIR_WEST, false));
        map_item_add_xy(map, X + WIDTH - 1, Y + 6, item_new_tex(OBJ_ID_SIGN_SQUARE, TEX_ID_PICTURE_3, DIR_EAST, false));

        // Entrance(s)
        add_room(map, X + 1, Y + HEIGHT, 2, 1);

        // Level 5 shutter
        add_door(map, X + 1 + 0, Y - 1, DIR_SOUTH, 2509);  // FIXME:
        map_tile_clone_xy(map, X + 1 + 1, Y - 1, X + 1 + 0, Y - 1);
    }

    //---------------------------------------------------------------------------
    // Room 1
    //---------------------------------------------------------------------------
    {
        const int X = 31;
        const int Y = 19;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X + WIDTH, Y + 2, 1, 2);
    }

    //---------------------------------------------------------------------------
    // Room 2
    //---------------------------------------------------------------------------
    {
        const int X = 31;
        const int Y = 26;
        const int WIDTH = 6;
        const int HEIGHT = 6;

        add_room(map, X, Y, WIDTH, HEIGHT);
        add_barrier(map, X + 2, Y + 2, 2, 2);

        // Entrance(s)
        add_room(map, X + WIDTH, Y + 2, 1, 2);
        add_room(map, X + 2, Y - 1, 2, 1);
    }

    //---------------------------------------------------------------------------
    // Room 3
    //---------------------------------------------------------------------------
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

    //---------------------------------------------------------------------------
    // Room 4
    //---------------------------------------------------------------------------
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
    //---------------------------------------------------------------------------
    // Level 5
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Big room
    //---------------------------------------------------------------------------
    {
        const int X = 34;
        const int Y = 6;

        add_room(map, X + 4, Y + 0, 4, 2);
        add_room(map, X + 2, Y + 2, 8, 2);
        add_room(map, X + 0, Y + 4, 12, 2);
        add_room(map, X + 0, Y + 6, 12, 2);
        add_room(map, X + 2, Y + 8, 8, 2);
        add_room(map, X + 4, Y + 10, 4, 2);

        // Entrance(s)
        add_room(map, X + 5, Y + 12, 2, 1);
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
