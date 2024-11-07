#include "mapgen/room.h"

minigl_matgroup_t mat_wall = {.size = 2, .color = {160, 128}};
minigl_matgroup_t mat_shutter = {.size = 3, .color = {160, 128, 96}};

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
    furnish_default(map, b);

    // Venus
    room_add_item(map, b, 4, 4, item_new_mat(OBJ_ID_BASE, &mat_wall, DIR_NORTH, true));
    room_add_item(map, b, 4, 4, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));

    // Columns
    // FIXME: Add mechanism to rotate ...
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
    add_picture(map, b, 3, 3, DIR_NORTH);
    add_picture(map, b, 5, 3, DIR_NORTH);

    add_picture(map, b, 3, 1, DIR_SOUTH);
    add_picture(map, b, 5, 1, DIR_SOUTH);
}

roomlib_room_t room_atrium = {9,
                              11,
                              &furnish_atrium,
                              3,
                              {
                                  {3, 3, DIR_NORTH},
                                  {5, 2, DIR_EAST},
                                  {5, 2, DIR_WEST},
                              }};

roomlib_room_t room_gallery = {5,
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

roomlib_room_t room_donut = {9,
                             5,
                             &furnish_donut,
                             2,
                             {
                                 {3, 3, DIR_NORTH},
                                 {3, 3, DIR_SOUTH},
                             }};

roomlib_room_t room0 = {9,
                        4,
                        NULL,
                        3,
                        {
                            {3, 3, DIR_NORTH},
                            {1, 2, DIR_EAST},
                            {3, 3, DIR_SOUTH},
                            {1, 2, DIR_WEST},
                        }};

roomlib_room_t room1 = {9,
                        9,
                        NULL,
                        4,
                        {
                            {3, 3, DIR_NORTH},
                            {1, 2, DIR_EAST},
                            {3, 3, DIR_SOUTH},
                            {1, 2, DIR_WEST},
                        }};

roomlib_room_t room2 = {4,
                        4,
                        NULL,
                        4,
                        {
                            {1, 2, DIR_NORTH},
                            {1, 2, DIR_EAST},
                            {1, 2, DIR_SOUTH},
                            {1, 2, DIR_WEST},
                        }};

roomlib_room_t room3 = {4,
                        9,
                        NULL,
                        6,
                        {
                            {1, 2, DIR_NORTH},
                            {1, 2, DIR_EAST},
                            {5, 2, DIR_EAST},
                            {1, 2, DIR_SOUTH},
                            {1, 2, DIR_WEST},
                            {5, 2, DIR_WEST},
                        }};

roomlib_room_t room4 = {9,
                        9,
                        NULL,
                        8,
                        {
                            {1, 2, DIR_NORTH},
                            {5, 2, DIR_NORTH},
                            {1, 2, DIR_EAST},
                            {5, 2, DIR_EAST},
                            {1, 2, DIR_SOUTH},
                            {5, 2, DIR_SOUTH},
                            {1, 2, DIR_WEST},
                            {5, 2, DIR_WEST},
                        }};

roomlib_room_t* room_library[] = {&room_gallery, &room0, &room1, &room_donut};

#define ROOM_NUM 4
