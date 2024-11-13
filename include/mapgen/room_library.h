#include "mapgen/room.h"
#include "random.h"

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
    // Don't fill the entire room!
    room_add_floor(map, b, 0, 0, b.width, b.height - 4);

    // Entrance
    room_add_floor(map, b, 3, b.height - 4, 3, 1);

    // Door
    // TODO: Generate pin
    room_add_door(map, b, 3, b.height - 4, 3, DIR_NORTH);

    // This section is normally unreachable
    for (int i = 0; i < 4; i++) {
        room_add_item(map, b, 2 * i, b.height - 2, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
    }

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < b.width; x++) {
            item_t* item = item_new_color(OBJ_ID_FLOOR, 64, DIR_NORTH, true);
            item->type = ITEM_FLOOR;
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

    for (int i = 0; i < 3; i++) {
        int t = randi_range(0, 4);
        int y_off = 1 + i * 5;
        switch (t) {
            case 0:
                // Vertical
                room_remove_floor(map, b, 3, y_off + 1, 1, 3);
                break;
            case 1:
                // Horizontal
                room_remove_floor(map, b, 2, y_off + 2, 3, 1);
                break;
            case 2:
                // Square
                room_remove_floor(map, b, 2, y_off + 1, 3, 3);
                break;
            case 3:
                // Bench
                room_add_item(map, b, 3, y_off + 2, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_WEST, b.r), true));
                break;
            case 4:
                // Sculpture
                add_sculpture(map, b, 3, y_off + 2);
                break;
        }
    }

    for (int i = 0; i < 7; i++) {
        // TODO: top walls?
        add_picture(map, b, 0, 2 + 2 * i, DIR_WEST);
        add_picture(map, b, 6, 2 + 2 * i, DIR_EAST);
    }
}

static void furnish_corridor(map_t* map, bounds_t b) {
    furnish_default(map, b);

    // Pictures
    for (int i = 0; i < 3; i++) {
        add_picture(map, b, 0, 1 + 2 * i, DIR_WEST);
        add_picture(map, b, 3, 1 + 2 * i, DIR_EAST);
    }
}

static void furnish_square_small(map_t* map, bounds_t b) {
    furnish_default(map, b);

    int t = randi_range(0, 2);
    switch (t) {
        case 0:
            // Bench
            room_add_item(map, b, 2, 2, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_WEST, b.r), true));
            break;
        case 1:
            // Sculpture
            add_sculpture(map, b, 2, 2);
            break;
    }

    for (int i = 0; i < 2; i++) {
        add_picture(map, b, 1 + i * 2, 0, DIR_NORTH);
        add_picture(map, b, 4, 1 + i * 2, DIR_EAST);
        add_picture(map, b, 1 + i * 2, 4, DIR_SOUTH);
        add_picture(map, b, 0, 1 + i * 2, DIR_WEST);
    }
}

static void furnish_square_large(map_t* map, bounds_t b) {
    furnish_default(map, b);

    switch (randi(2)) {
        case 0:
            // Columns
            room_add_item(map, b, 2, 2, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
            room_add_item(map, b, 2, 6, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
            room_add_item(map, b, 6, 2, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
            room_add_item(map, b, 6, 6, item_new_color(OBJ_ID_COLUMN, 100, DIR_NORTH, true));
            switch (randi(2)) {
                case 0:
                    // Statues
                    add_sculpture(map, b, 4, 3);
                    add_sculpture(map, b, 4, 5);
                    add_sculpture(map, b, 3, 4);
                    add_sculpture(map, b, 5, 4);
                    break;
                case 1:
                    // Benches
                    add_sculpture(map, b, 4, 4);
                    room_add_item(map, b, 4, 2, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_NORTH, b.r), true));
                    room_add_item(map, b, 4, 6, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_NORTH, b.r), true));
                    room_add_item(map, b, 2, 4, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_WEST, b.r), true));
                    room_add_item(map, b, 6, 4, item_new_color(OBJ_ID_BENCH, 100, dir_rotate(DIR_WEST, b.r), true));
                    break;
            }
            break;
        case 1:
            // Square
            room_remove_floor(map, b, 3, 3, 3, 3);
            break;
    }
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
    7,
    17,
    &furnish_gallery,
    6,
    {
        {2, 3, DIR_NORTH},
        {2, 3, DIR_SOUTH},
        {1, 2, DIR_EAST},
        {1, 2, DIR_WEST},
        {14, 2, DIR_EAST},
        {14, 2, DIR_WEST},
    }};

roomlib_room_t room_corridor = {
    4,
    7,
    &furnish_corridor,
    2,
    {
        {1, 2, DIR_NORTH},
        {1, 2, DIR_SOUTH},
    }};

roomlib_room_t room_square_small = {
    5,
    5,
    &furnish_square_small,
    5,
    {
        {1, 3, DIR_NORTH},
        {2, 2, DIR_NORTH},
        {1, 3, DIR_EAST},
        {2, 2, DIR_EAST},
        {1, 3, DIR_WEST}
    }};

roomlib_room_t room_square_large = {
    9,
    9,
    &furnish_square_large,
    4,
    {
        {3, 3, DIR_NORTH},
        {3, 3, DIR_SOUTH},
        {3, 3, DIR_EAST},
        {3, 3, DIR_WEST},
    }};

roomlib_room_t* room_library[] = {
    &room_gallery, 
    &room_corridor,
    &room_square_small,
    &room_square_large,
};
// clang-format on

#define ROOM_NUM 4
