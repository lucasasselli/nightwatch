#include "map_generator.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

map_room_t *rooms;
int room_cnt;
// clang-format off

const randw_constr_int_t CONSTR_ROOM_T = {
    .size = 3,
    .values = {
        {ROOM_BASE, 8},
        {ROOM_CORRIDOR_NS, 1},
        {ROOM_CORRIDOR_EW, 1}
    }
};

const randw_constr_int_t CONSTR_EXIT_CNT = {
    .size = 5,
    .values = {
        {.value = 1, .weight = 2}, 
        {.value = 2, .weight = 8}, 
        {.value = 3, .weight = 4}, 
        {.value = 4, .weight = 2}
    }
};
// clang-format on

static char tile_to_char(map_tile_t tile) {
    if (tile.item_cnt == 0) {
        return ' ';
    } else if (tile.item_cnt > 1) {
        return '+';
    } else {
        switch (tile.items[0].type) {
            case ITEM_FLOOR:
                return '.';
            case ITEM_STATUE:
                return 'o';
            case ITEM_WALL:
                return '#';
        }
    }
}

static bool tile_has_item(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE) return false;
    map_tile_t *tile = &map[y][x];
    for (int i = 0; i < tile->item_cnt; i++) {
        map_item_t item = tile->items[i];
        if (item.type == type && (item.dir == dir || dir == DIR_ANY)) return true;
    }
    return false;
}

static bool tile_remove_item(map_t map, map_item_t query, int x, int y) {
    if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE) return false;
    map_tile_t *tile = &map[y][x];
    int j = 0;
    bool found = false;
    for (int i = 0; i < tile->item_cnt; i++) {
        map_item_t item = tile->items[i];
        if (item.type == query.type && (item.dir == query.dir)) {
            found = true;
            continue;
        }

        tile->items[j] = tile->items[i];
        j++;
    }

    tile->item_cnt = j;
    return found;
}

static void tile_item_add(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    map_item_t item;
    item.type = type;
    item.dir = dir;
    map_tile_t *tile = &map[y][x];

    if (item.type == ITEM_WALL) {
        switch (item.dir) {
            case DIR_NORTH:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_SOUTH}, x, y - 1)) {
                    return;
                }
                break;

            case DIR_EAST:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_WEST}, x + 1, y)) {
                    return;
                }
                break;

            case DIR_SOUTH:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_NORTH}, x, y + 1)) {
                    return;
                }
                break;

            case DIR_WEST:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_EAST}, x - 1, y)) {
                    return;
                }
                break;
        }
    } else if (item.type == ITEM_STATUE) {
        tile->collide = true;
    }

    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
}

static bool mapgen_room_check_oob(map_room_t room) {
    if (room.pos[0] < 0 || (room.pos[0] + room.size[0] >= MAP_SIZE)) return true;
    if (room.pos[1] < 0 || (room.pos[1] + room.size[1] >= MAP_SIZE)) return true;
    return false;
}

static bool mapgen_room_check_collision(map_room_t room) {
    // Check against map boundaries
    if (mapgen_room_check_oob(room)) return false;

    // Check against other rooms
    bool match = false;

    for (int i = 0; i < room_cnt; i++) {
        map_room_t other = rooms[i];

        if ((room.pos[0] + room.size[0]) <= other.pos[0]) {
            continue;
        }

        if (room.pos[0] >= (other.pos[0] + other.size[0])) {
            continue;
        }

        if ((room.pos[1] + room.size[1]) <= other.pos[1]) {
            continue;
        }

        if (room.pos[1] >= (other.pos[1] + other.size[1])) {
            continue;
        }

        match = true;
    }

    return !match;
}

static void grid_add_item_col(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, dir, x, y + i);
    }
}

static void grid_add_item_row(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, dir, x + i, y);
    }
}

void mapgen_grid_update(map_t map) {
    for (int i = 0; i < room_cnt; i++) {
        map_room_t room = rooms[i];

        assert((room.pos[1] + room.size[1]) < MAP_SIZE);
        assert((room.pos[0] + room.size[0]) < MAP_SIZE);

        // Floor
        for (int y = 0; y < room.size[1]; y++) {
            for (int x = 0; x < room.size[0]; x++) {
                tile_item_add(map, ITEM_FLOOR, DIR_ANY, room.pos[0] + x, room.pos[1] + y);
            }
        }

        // Walls
        grid_add_item_row(map, ITEM_WALL, DIR_NORTH, room.pos[0], room.pos[1], room.size[0]);
        grid_add_item_col(map, ITEM_WALL, DIR_EAST, room.pos[0] + room.size[0] - 1, room.pos[1], room.size[1]);
        grid_add_item_row(map, ITEM_WALL, DIR_SOUTH, room.pos[0], room.pos[1] + room.size[1] - 1, room.size[0]);
        grid_add_item_col(map, ITEM_WALL, DIR_WEST, room.pos[0], room.pos[1], room.size[1]);

        // Add statues
        if (room.type != ROOM_CORRIDOR_NS && room.type != ROOM_CORRIDOR_EW) {
            // Add statues based on room size
            int statue_cnt = sqrt(room.size[0] * room.size[1]) / 2;

            for (int i = 0; i < statue_cnt; i++) {
                int x, y;

                // TODO: Create either at margin or center of the room
                do {
                    x = rand_range(room.pos[0], room.pos[0] + room.size[0]);
                    y = rand_range(room.pos[1], room.pos[1] + room.size[1]);
                } while (tile_has_item(map, ITEM_STATUE, DIR_ANY, x, y));

                tile_item_add(map, ITEM_STATUE, rand() % 4, x, y);
            }
        }
    }
}

void mapgen_grid_print(map_t map) {
    char buf[MAP_SIZE + 1];
    buf[MAP_SIZE] = '\0';
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            buf[x] = tile_to_char(map[y][x]);
        }
        debug("%s", buf);
    }
}

static map_room_t room_randomize(map_room_t prev) {
    map_room_t out;

    //---------------------------------------------------------------------------
    // Pick a random room
    //---------------------------------------------------------------------------

    if (prev.type == ROOM_BASE) {
        out.type = randwi(&CONSTR_ROOM_T);
    } else {
        // Don't do back to back corridors
        out.type = ROOM_BASE;
    }

    switch (out.type) {
        case ROOM_BASE:
            randi_array(out.size, 2, 5, 8);
            break;

        case ROOM_CORRIDOR_NS:
            // North-south
            out.size[0] = 3;
            out.size[1] = randi(7, 10);
            break;

        case ROOM_CORRIDOR_EW:
            // East-west
            out.size[0] = randi(7, 10);
            out.size[1] = 3;
            break;
    }

    //---------------------------------------------------------------------------
    // Calculate position
    //---------------------------------------------------------------------------

    const int ROOM_MARGIN = 2;
    const int DOOR_MARGIN = 1;

    //     ┌─────┐
    //     │Next │
    //     │Room │
    //     └─────┘
    //
    //       min_y
    // min_x┌───────────────────┐max_x
    //      │                   │
    //      │                   │
    //      │     ┌─────────────┤
    //      │     │             │
    //      │     │             │
    //      │     │    This     │
    //      │     │    Room     │
    //      │     │             │
    //      │     │             │
    //      │     │             │
    //      └─────┴─────────────┘
    //       max_y
    //
    // Next room will be randomized on the outher borders of
    // The perimeter of the big triangle, with a inner padding
    // of 2 to allow to fit the door!

    // Calculate the min and max coordinates for next room origin
    int min_x = prev.pos[0] - (out.size[0]);
    int max_x = prev.pos[0] + (prev.size[0]);
    int min_y = prev.pos[1] - (out.size[1]);
    int max_y = prev.pos[1] + (prev.size[1]);

    do {
        // Pick a side for the next room
        int side;

        switch (out.type) {
            case ROOM_BASE:
                side = randi(DIR_NORTH, DIR_WEST + 1);
                break;

            case ROOM_CORRIDOR_NS:
                // North-south
                side = randi(0, 2) ? 0 : 2;
                break;
            case ROOM_CORRIDOR_EW:
                side = randi(0, 2) ? 1 : 3;
                break;
        }

        // Check that the side can fit a room and a door
        if (side % 2 == 0) {
            // North - south
            if (max_x - min_x < (ROOM_MARGIN * 2)) continue;
        } else {
            // East - west
            if (max_y - min_y < (ROOM_MARGIN * 2)) continue;
        }

        if (side == 0 || side == 2) {
            // North - South
            out.pos[0] = rand_range(min_x + ROOM_MARGIN, max_x - ROOM_MARGIN);

            int door_min = maxi(prev.pos[0] + DOOR_MARGIN, out.pos[0] + DOOR_MARGIN);
            int door_max = mini(prev.pos[0] + prev.size[0] - DOOR_MARGIN, out.pos[0] + out.size[0] - DOOR_MARGIN);

            if (door_max - door_min == 0) continue;

            if (side == 0) {
                // North
                out.pos[1] = min_y;
            } else {
                // South
                out.pos[1] = max_y;
            }
        }

        if (side == 1 || side == 3) {
            // East - West
            out.pos[1] = rand_range(min_y + ROOM_MARGIN, max_y - ROOM_MARGIN);

            int door_min_y = maxi(prev.pos[1] + DOOR_MARGIN, out.pos[1] + DOOR_MARGIN);
            int door_max_y = mini(prev.pos[1] + prev.size[1] - DOOR_MARGIN, out.pos[1] + out.size[1] - DOOR_MARGIN);

            if (door_max_y - door_min_y == 0) continue;

            if (side == 1) {
                // East
                out.pos[0] = max_x;
            } else {
                // West
                out.pos[0] = min_x;
            }
        }

        break;
    } while (true);

    return out;
}

static void room_add(map_room_t room) {
    assert(room_cnt < MAP_MAX_ROOMS);
    rooms[room_cnt] = room;
    room_cnt++;
}

static void room_gen_next(map_t map, map_room_t room) {
    // Pick a point on the room walls and use this to generate the next room
    int exit_cnt = randwi(&CONSTR_EXIT_CNT);

    for (int i = 0; i < exit_cnt; i++) {
        if (room_cnt >= (MAP_MAX_ROOMS - 1)) {
            return;
        }

        map_room_t next_room;
        int tries = 0;
        do {
            next_room = room_randomize(room);
            tries++;
        } while (!mapgen_room_check_collision(next_room) && tries < MAP_RAND_EFFORT);

        if (tries >= MAP_RAND_EFFORT) {
            debug("Max effort reached!\n");
            return;
        }

        room_add(next_room);
        room_gen_next(map, next_room);
    }
}

void mapgen_gen(map_t map) {
    // Initialize
    rooms = malloc(sizeof(map_room_t) * MAP_MAX_ROOMS);
    room_cnt = 0;

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // Initialize the tile
            map[y][x].item_cnt = 0;  // No item by default
            map[y][x].items = malloc(sizeof(map_item_t) * MAP_TILE_MAX_ITEMS);
        }
    }

    // Generate the first room
    map_room_t room;

    room.size[0] = 4;
    room.size[1] = 4;

    // Put first room at center of map
    room.pos[0] = MAP_SIZE / 2 - room.size[0] / 2;
    room.pos[1] = MAP_SIZE / 2 - room.size[1] / 2;

    room_add(room);

    if (MAP_MAX_ROOMS > 1) {
        // Create adjacent rooms
        room_gen_next(map, room);
    }

    // Update the grid
    mapgen_grid_update(map);

    // Cleanup
    free(rooms);
}
