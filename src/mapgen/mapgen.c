#include "mapgen/mapgen.h"

#include <stdlib.h>

#include "data/cbuff.h"
#include "item.h"
#include "mapgen/room.h"
#include "mapgen/room_library.h"
#include "random.h"

#define MAPGEN_ROOMS_MAX 20

static void map_add_floor(map_t* map, int pos_x, int pos_y, int width, int height) {
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

static void gen_walls(map_t* map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (tile_has_item(map->grid[y][x], ITEM_FLOOR, -1, -1)) {
                if (!tile_has_item(map->grid[y - 1][x], -1, -1, -1)) {
                    add_wall(map, x, y - 1, DIR_SOUTH);
                }
                if (!tile_has_item(map->grid[y][x + 1], -1, -1, -1)) {
                    add_wall(map, x + 1, y, DIR_WEST);
                }
                if (!tile_has_item(map->grid[y + 1][x], -1, -1, -1)) {
                    add_wall(map, x, y + 1, DIR_NORTH);
                }
                if (!tile_has_item(map->grid[y][x - 1], -1, -1, -1)) {
                    add_wall(map, x - 1, y, DIR_EAST);
                }
            }
        }
    }
}

static cbuff_t* doors_cbuff;
static bounds_t* room_array;
static furnish_fun_t* furnish_array;
static int room_cnt;

static bool can_place(bounds_t x) {
    bounds_t r1 = room_norm_r(x);

    if (x.x < 1 || x.y < 1 || x.x + x.width >= MAP_SIZE - 1 || x.y + x.height >= MAP_SIZE - 1) return false;

    const int MAX_DIST = 1;

    for (int i = 0; i < room_cnt; i++) {
        bounds_t r2 = room_norm_r(room_array[i]);
        if ((r1.x + r1.width + MAX_DIST) <= r2.x) {
            continue;
        }

        if (r1.x >= (r2.x + r2.width + MAX_DIST)) {
            continue;
        }

        if ((r1.y + r1.height + MAX_DIST) <= r2.y) {
            continue;
        }

        if (r1.y >= (r2.y + r2.height + MAX_DIST)) {
            continue;
        }

        return false;
    }

    return true;
}

static void add_room(roomlib_room_t* room, int x, int y, int r) {
    int w;
    int h;

    if (r % 2) {
        w = room->height;
        h = room->width;
    } else {
        h = room->height;
        w = room->width;
    }

    // Add doors
    for (int i = 0; i < room->door_num; i++) {
        dir_t dir = dir_rotate(room->doors[i].dir, r);

        int door_x;
        int door_y;
        switch (dir) {
            case DIR_NORTH:
                door_x = x + room->doors[i].offset;
                door_y = y - 1;
                break;
            case DIR_EAST:
                door_x = x + w;
                door_y = y + room->doors[i].offset;
                break;
            case DIR_SOUTH:
                door_x = x + room->doors[i].offset;
                door_y = y + h;
                break;
            case DIR_WEST:
                door_x = x - 1;
                door_y = y + room->doors[i].offset;
                break;
        }

        // TODO: Before adding a door, check if it overlaps with an already placed door
        door_t door;
        door.x = door_x;
        door.y = door_y;
        door.dir = dir;
        door.size = room->doors[i].size;

        cbuff_push(doors_cbuff, &door);
    }

    room_array[room_cnt] = ((bounds_t){x, y, room->width, room->height, r});

    if (room->furnish == NULL) {
        furnish_array[room_cnt] = &furnish_default;
    } else {
        furnish_array[room_cnt] = room->furnish;
    }
    room_cnt++;

    cbuff_shuffle(doors_cbuff, 3);
}

static bool add_room_at_door(map_t* map, roomlib_room_t* room, door_t door_out) {
    // Pick a random door
    int rand_door_start = randi(0, room->door_num);

    for (int i = 0; i < room->door_num; i++) {
        roomlib_door_t door_in = room->doors[(rand_door_start + i) % room->door_num];

        // Does the door size match?
        if (door_in.size == door_out.size) {
            // Calculate rotation
            int r = dir_delta(door_out.dir, dir_flip(door_in.dir));

            int w;
            int h;

            if (r % 2) {
                w = room->height;
                h = room->width;
            } else {
                h = room->height;
                w = room->width;
            }

            // Calculate the room position based on where the exit was pointing
            // towards
            int x;
            int y;
            switch (door_out.dir) {
                case DIR_NORTH:
                    x = door_out.x - door_in.offset;
                    y = door_out.y - h;
                    break;
                case DIR_EAST:
                    x = door_out.x + 1;
                    y = door_out.y - door_in.offset;
                    break;
                case DIR_SOUTH:
                    x = door_out.x - door_in.offset;
                    y = door_out.y + 1;
                    break;
                case DIR_WEST:
                    x = door_out.x - w;
                    y = door_out.y - door_in.offset;
                    break;
            }

            // Check if the rotated room fits
            if (can_place((bounds_t){x, y, room->width, room->height, r})) {
                // Add door
                switch (door_out.dir) {
                    case DIR_NORTH:
                    case DIR_SOUTH:
                        map_add_floor(map, door_out.x, door_out.y, door_out.size, 1);
                        break;
                    case DIR_EAST:
                    case DIR_WEST:
                        map_add_floor(map, door_out.x, door_out.y, 1, door_out.size);
                        break;
                }

                // Add room
                add_room(room, x, y, r);
                return true;
            }
        }
    }

    return false;
}

void mapgen_gen(map_t* map) {
    room_cnt = 0;

    doors_cbuff = cbuff_new(sizeof(door_t), 100);
    room_array = malloc(sizeof(bounds_t) * MAPGEN_ROOMS_MAX);
    furnish_array = malloc(sizeof(furnish_fun_t*) * MAPGEN_ROOMS_MAX);

    // Place start room
    add_room(&room_atrium, MAP_SIZE / 2 - room_atrium.width / 2, MAP_SIZE - room_atrium.height - 2, 0);

    // Foreach door in list place a room
    while (cbuff_size(doors_cbuff) > 0) {
        door_t door;
        cbuff_pop(doors_cbuff, &door);

        // Pick a random room
        int rand_room_start = randi(0, ROOM_NUM);
        for (int i = 0; i < ROOM_NUM; i++) {
            // Cycle the room library
            roomlib_room_t* room = room_library[(rand_room_start + i) % ROOM_NUM];
            if (add_room_at_door(map, room, door)) {
                break;
            }
        }

        if (room_cnt >= MAPGEN_ROOMS_MAX) break;
    }

    // Add the furniture
    for (int i = 0; i < room_cnt; i++) {
        (*furnish_array[i])(map, room_array[i]);
    }

    // Automatically add walls
    gen_walls(map);

    cbuff_free(doors_cbuff);
    free(furnish_array);
    free(room_array);
}
