#include "map_reader.h"

minigl_matgroup_t mat_wall = {.size = 2, .color = {160, 128}};

static void add_room(map_t map, int pos_x, int pos_y, int width, int height) {
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

static void add_door(map_t map, int x, int y, dir_t dir, int pin) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->obj_id = OBJ_ID_WALL;
    item->tex_id = TEX_ID_FENCE_CLOSED;
    item->draw_mode = DRAW_MODE_COLOR;
    item->dir = dir;
    item->action.type = ACTION_KEYPAD;
    item->action.arg = pin;
    map_item_add_xy(map, x, y, item);
}

static void add_note(map_t map, int x, int y, int id) {
    item_t* item = item_new_tex(OBJ_ID_NOTE, TEX_ID_NOTE, DIR_NORTH);
    item->effects = EFFECT_SPIN;
    item->action.type = ACTION_NOTE;
    item->action.arg = id;
    map_item_add_xy(map, x, y, item);
}

static void add_wall(map_t map, int x, int y, dir_t dir) {
    item_t* item = item_new();
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

void map_read(map_t map) {
    // TODO: Implement read from file?

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

        // Entrance
        add_room(map, X, Y, WIDTH, HEIGHT);
        add_door(map, 30, 60, DIR_NORTH, 1234);
        add_door(map, 31, 60, DIR_NORTH, 1234);
        add_door(map, 32, 60, DIR_NORTH, 1234);

        // Columns
        map_item_add_xy(map, X + 2, Y + 2 + 0, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y + 2 + 0, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_7, DIR_SOUTH));

        map_item_add_xy(map, X + 2, Y + 2 + 3, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y + 2 + 3, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_3, DIR_SOUTH));

        map_item_add_xy(map, X + 2, Y + 2 + 6, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y + 2 + 6, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_0, DIR_SOUTH));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 0, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 0, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_6, DIR_SOUTH));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 3, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 3, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_1, DIR_SOUTH));

        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 6, item_new_mat(OBJ_ID_COLUMN, &mat_wall, DIR_NORTH));
        map_item_add_xy(map, X + WIDTH - 3, Y + 2 + 6, item_new_tex(OBJ_ID_COLUMN_GRAFFITI, TEX_ID_COLUMN_DIG_4, DIR_SOUTH));

        // Statue
        // TODO: Replace with something else
        map_item_add_xy(map, X + 4, Y + 4 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH));

        // Level 2 gate
        add_room(map, X + 3, Y - 1, 3, 1);
        add_door(map, X + 3 + 0, Y - 1, DIR_SOUTH, 4673);
        add_door(map, X + 3 + 1, Y - 1, DIR_SOUTH, 4673);
        add_door(map, X + 3 + 2, Y - 1, DIR_SOUTH, 4673);

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
        map_item_add_xy(map, X - 2, Y + 1, item_new_tex(OBJ_ID_WC_SIGN, TEX_ID_SIGN_WC_MAN, DIR_EAST));

        // Barrier
        map_item_add_xy(map, X - 1, Y, item_new_tex(OBJ_ID_WETFLOOR, TEX_ID_WETFLOOR, DIR_WEST));
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
        map_item_add_xy(map, X - 2, Y + HEIGHT, item_new_tex(OBJ_ID_WC_SIGN, TEX_ID_SIGN_WC_WOMEN, DIR_EAST));

        // Stalls
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH));
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH));
        map_item_add_xy(map, X + 1, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH));
        map_item_add_xy(map, X + 1, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_SOUTH));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_COLUMN, 96, DIR_SOUTH));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_WC_PANEL, 32, DIR_EAST));

        map_item_add_xy(map, X + 3, Y, item_new_tex(OBJ_ID_WALL_GRAFFITI, TEX_LVL1_CYPHER, DIR_WEST));

        // Sinks
        map_item_add_xy(map, X + 3, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH));
        map_item_add_xy(map, X + 4, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH));
        map_item_add_xy(map, X + 5, Y, item_new_tex(OBJ_ID_WC_SINK, TEX_ID_WC_SINK, DIR_NORTH));

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

        map_item_add_xy(map, X + 2 + 0, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH));
        map_item_add_xy(map, X + 2 + 2, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH));
        map_item_add_xy(map, X + 2 + 4, Y + 2 + 0, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH));
        map_item_add_xy(map, X + 2 + 0, Y + 2 + 2, item_new_mdbb(TEX_ID_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 2, Y + 2 + 2, item_new_mdbb(TEX_ID_VENUS, DIR_NORTH));
        map_item_add_xy(map, X + 2 + 4, Y + 2 + 2, item_new_mdbb(TEX_ID_VENUS, DIR_NORTH));
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
        map_item_add_xy(map, X + 2, Y + 3, item_new_tex(OBJ_ID_BENCH, TEX_ID_WOOD, DIR_WEST));
        map_item_add_xy(map, X + 2, Y + 7, item_new_tex(OBJ_ID_BENCH, TEX_ID_WOOD, DIR_WEST));
        map_item_add_xy(map, X + 2, Y + 11, item_new_tex(OBJ_ID_BENCH, TEX_ID_WOOD, DIR_WEST));
    }

    //---------------------------------------------------------------------------
    // Automatically add walls
    //---------------------------------------------------------------------------

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

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    // Outside
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------

    {
        const int X = 28;
        const int Y = 62;
        const int WIDTH = 8;
        const int HEIGHT = 3;

        // This section is normally unreachable
        map_item_add_xy(map, X + 0, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + 2, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + 4, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH));
        map_item_add_xy(map, X + 6, Y, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH));

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                map_item_add_xy(map, X + x, 62 - y, item_new_color(OBJ_ID_FLOOR, 64, DIR_NORTH));
            }
        }
    }
}
