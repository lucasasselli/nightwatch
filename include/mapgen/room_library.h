#include "mapgen/room.h"

minigl_matgroup_t mat_wall = {.size = 2, .color = {160, 128}};
minigl_matgroup_t mat_random = {.size = 5, .color = {0, 64, 128, 192, 255}};

typedef void (*furnish_fun_t)(map_t*, bounds_t);

typedef struct {
    int offset;
    int size;
    dir_t dir;
} roomlib_door_t;

typedef struct {
    int width;
    int height;
    furnish_fun_t furnish;
    int door_num;
    roomlib_door_t doors[];
} roomlib_room_t;

static void furnish_default(map_t* map, bounds_t b) {
    room_add_floor(map, b, 0, 0, b.width, b.height);
}

static void furnish_atrium(map_t* map, bounds_t b) {
    const int OUTSIDE_Y = 2;

    // Don't fill the entire room!
    room_add_floor(map, b, 0, 0, b.width, b.height - 4);

    // Entrance
    room_add_floor(map, b, 3, b.height - 4, 3, 1);

    // Door
    // TODO: Generate pin
    room_add_door(map, b, 3, b.height - 4, 3, DIR_NORTH);

    // This section is normally unreachable
    for (int i = 0; i < 4; i++) {
        room_add_item(map, b, 2 * i, b.height - 1, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
    }

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < b.width; x++) {
            item_t* item = item_new_color(OBJ_ID_FLOOR, 64, DIR_NORTH, true);
            item->type = ITEM_NORMAL;
            room_add_item(map, b, x, b.height - 3 + y, item);
        }
    }

    // Venus
    room_add_item(map, b, 4, 4, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));
    room_add_item(map, b, 4, 4, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));

    // Columns
    for (int i = 0; i < 3; i++) {
        room_add_item(map, b, 2, 2 + i * 3, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
        room_add_item(map, b, b.width - 3, 2 + i * 3, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
    }
}

static void furnish_gallery(map_t* map, bounds_t b) {
    furnish_default(map, b);

    // Benches
    for (int i = 0; i < 3; i++) {
        room_add_item(map, b, 2, 3 + 4 * i, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_WEST, b.r), true));
    }

    // Pictures
    for (int i = 0; i < 6; i++) {
        add_picture(map, b, 0, 2 + 2 * i, DIR_WEST);
        add_picture(map, b, 4, 2 + 2 * i, DIR_EAST);
    }
}

static void furnish_donut(map_t* map, bounds_t b) {
    room_add_floor(map, b, 0, 0, 9, 2);
    room_add_floor(map, b, 0, 2, 2, 1);
    room_add_floor(map, b, 7, 2, 2, 1);
    room_add_floor(map, b, 0, 3, 9, 2);

    // Pictures
    for (int i = 0; i < 3; i++) {
        add_picture(map, b, 3 + i, 3, DIR_NORTH);
        add_picture(map, b, 3 + i, 1, DIR_SOUTH);
    }
}

static void furnish_square_small(map_t* map, bounds_t b) {
    furnish_default(map, b);

    for (int i = 0; i < 2; i++) {
        add_picture(map, b, 1 + i, 0, DIR_NORTH);
        add_picture(map, b, 3, 1 + i, DIR_EAST);
        add_picture(map, b, 1 + i, 3, DIR_SOUTH);
        add_picture(map, b, 0, 1 + i, DIR_WEST);
    }
}

static void furnish_square_large(map_t* map, bounds_t b) {
    furnish_default(map, b);

    add_sculpture(map, b, 2, 2);

    for (int i = 0; i < 3; i++) {
        add_picture(map, b, 1 + i, 0, DIR_NORTH);
        add_picture(map, b, 4, 1 + i, DIR_EAST);
        add_picture(map, b, 1 + i, 4, DIR_SOUTH);
        add_picture(map, b, 0, 1 + i, DIR_WEST);
    }
}

static void furnish_columns(map_t* map, bounds_t b) {
    furnish_default(map, b);

    room_add_item(map, b, 1, 1, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
    add_sculpture(map, b, 4, 1);

    add_sculpture(map, b, 1, 4);
    room_add_item(map, b, 4, 4, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));

    room_add_item(map, b, 1, 7, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
    add_sculpture(map, b, 4, 7);
}

// clang-format off
roomlib_room_t room_atrium = {
    9,
    15,
    &furnish_atrium,
    3,
    {
        {3, 3, DIR_NORTH},
        {5, 2, DIR_EAST},
        {5, 2, DIR_WEST},
    }};

roomlib_room_t room_gallery = {
    5,
    15,
    &furnish_gallery,
    6,
    {
        {1, 3, DIR_NORTH},
        {1, 2, DIR_WEST},
        {12, 2, DIR_WEST},
        {1, 3, DIR_SOUTH},
        {1, 2, DIR_EAST},
        {12, 2, DIR_EAST},
    }};

roomlib_room_t room_donut = {
    9,
    5,
    &furnish_donut,
    2,
    {
        {3, 3, DIR_NORTH},
        {3, 3, DIR_SOUTH},
    }};

roomlib_room_t room_square_small = {
    4,
    4,
    &furnish_square_small,
    4,
    {
        {1, 2, DIR_NORTH},
        {1, 2, DIR_EAST},
        {1, 2, DIR_SOUTH},
        {1, 2, DIR_WEST},
    }};

roomlib_room_t room_square_large = {
    5,
    5,
    &furnish_square_large,
    2,
    {
        {1, 3, DIR_NORTH},
        {1, 3, DIR_SOUTH}
    }};

roomlib_room_t room_columns = {
    6,
    9,
    furnish_columns,
    2,
    {
        {2, 2, DIR_NORTH},
        {2, 2, DIR_SOUTH},
    }};

roomlib_room_t* room_library[] = {
    &room_gallery, 
    &room_donut,
    &room_square_small,
    &room_square_large,
    &room_columns,
};
// clang-format on

#define ROOM_NUM 5
